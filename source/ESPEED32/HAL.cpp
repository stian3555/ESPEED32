/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include "HAL.h"
#include "slot_ESC.h"
#include <math.h>

/* Include appropriate sensor library based on selection */
#ifdef AS5600_MAG
  #include "AS5600.h"
  AS5600 as5600;  /* AS5600 magnetic sensor instance */

#elif defined(AS5600L_MAG)
  #include "AS5600L.h"
  AS5600L as5600;  /* AS5600L magnetic sensor instance (different I2C address) */

#elif defined(MT6701_MAG)
  #include "MT6701.hpp"
  MT6701 mt6701;  /* MT6701 magnetic sensor instance */

#elif defined(TLE493D_MAG)
  #define ADDRESS 0x44              /* TLE493D-W2B6 A3 variant I2C address */
  #define MOD1_REG 0x11             /* MOD1 register address for A3 variant */
  #define MOD1_CONFIG 0b11110111    /* 7-byte read mode, fast mode, low power disabled */
  #include <Wire.h>

#endif

/*********************************************************************************************************************/
/*                                            Function Implementations                                              */
/*********************************************************************************************************************/

/**
 * @brief Initialize hardware components
 * @details Sets up serial communication, I2C, and PWM channels
 */
void HAL_InitHW() {
  /* Initialize serial for debugging */
  Serial.begin(115200);

  /* Configure ADC for current sensing on GPIO25 */
  analogSetAttenuation(ADC_11db);  /* Set ADC range to 0-3.3V */
  pinMode(HB_AN_PIN, INPUT);       /* Explicitly set pin as input */

#ifdef TLE493D_MAG
  /* Initialize I2C for TLE493D sensor */
  Wire1.begin(SDA0_PIN, SCL0_PIN, 100000L);
  delay(100);  /* Wait for I2C stabilization */
  
  /* Configure TLE493D sensor */
  Wire1.beginTransmission(ADDRESS);
  Wire1.write(MOD1_REG);
  Wire1.write(MOD1_CONFIG);
  Wire1.endTransmission();
#endif

  /* Configure motor control PWM channels */
  ledcAttachChannel(HB_IN_PIN, PWM_FREQ_DEFAULT * 1000, THR_PWM_RES_BIT, THR_IN_PWM_CHAN);
  ledcAttachChannel(HB_INH_PIN, PWM_FREQ_DEFAULT * 1000, THR_PWM_RES_BIT, THR_INH_PWM_CHAN);
}

/**
 * @brief Write PWM value to motor control channel
 * @param pwmChan PWM channel number
 * @param value PWM duty cycle value (0-255)
 * @note Adapted for ESP32 3.0.0 library (ledcWrite takes PIN, not CHANNEL)
 */
void HALanalogWrite(const int pwmChan, int value) {
  switch (pwmChan) {
    case THR_IN_PWM_CHAN:
      ledcWrite(HB_IN_PIN, (uint32_t)value);
      break;

    case THR_INH_PWM_CHAN:
      ledcWrite(HB_INH_PIN, (uint32_t)value);
      break;
    
    default:
      break;
  }
}

/**
 * @brief Read raw trigger value from configured sensor
 * @return Raw trigger value (sensor-dependent scale)
 */
int16_t HAL_ReadTriggerRaw() {
  uint16_t retVal = 0;

  #if defined(AS5600_MAG) || defined(AS5600L)
    retVal = as5600.readAngle();

  #elif defined(MT6701_MAG)
    retVal = mt6701.getAngleDegrees();

  #elif defined(ANALOG_TRIG)
    retVal = analogRead(AN_THROT_PIN);

  #elif defined(TLE493D_MAG)
    byte data[7];
    
    /* Read 7 bytes from sensor */
    Wire1.requestFrom(ADDRESS, 7);
    for (byte i = 0; i < 7; i++) {
      data[i] = Wire1.read();
    }
    
    /* Extract X and Y magnetic field components */
    int16_t x = (data[0] << 4) | (data[4] >> 4);
    if (x >= 2048) x -= 4096;
    
    int16_t y = (data[1] << 4) | (data[4] & 0x0F);
    if (y >= 2048) y -= 4096;
    
    /* Apply simple moving average filter */
    static int16_t x_avg = 0, y_avg = 0;
    x_avg = (x_avg * 3 + x) / 4;
    y_avg = (y_avg * 3 + y) / 4;
    
    /* Calculate angle from magnetic field components */
    float angleRad = atan2((float)y_avg, (float)x_avg);
    float angleDeg = angleRad * 180.0 / PI;
    if (angleDeg < 0) angleDeg += 360.0;
    
    retVal = (int16_t)(angleDeg * 10.0);  /* Return angle * 10 for extra precision */
    
  #endif

  return retVal;
}


/**
 * @brief Setup GPIO pins
 */
void HAL_PinSetup() {
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
}

/**
 * @brief Read voltage from voltage divider circuit
 * @param analogInput ADC pin number
 * @param rvfbl Lower resistor value [Ohm]
 * @param rvfbh Upper resistor value [Ohm]
 * @return Voltage applied to voltage divider [mV]
 */
uint16_t HAL_ReadVoltageDivider(int analogInput, uint32_t rvfbl, uint32_t rvfbh) {
  uint32_t adcRaw = analogRead(analogInput);
  
  /* Calculate voltage at ADC pin */
  uint32_t voltage = (ACD_VOLTAGE_RANGE_MVOLTS * adcRaw) / ACD_RESOLUTION_STEPS;
  
  /* Calculate voltage applied to voltage divider */
  voltage = (voltage * (rvfbl + rvfbh)) / rvfbl;
  
  return voltage;
}

/**
 * @brief Read motor current from BTN9960LV IS pin
 * @details Hardware: IS → 2.2kΩ → GND, IS → 2.2kΩ → D25, D25 → 100nF → GND
 *          BTN9960LV: IS = ILOAD / kILIS where kILIS ≈ 8500
 *          Voltage divider gives: V_ADC = (ILOAD / 8500) * 2200 / 2
 *          Therefore: ILOAD = V_ADC * 7752 mA
 * @return Motor current in milliamps [mA]
 */
uint16_t HAL_ReadMotorCurrent() {
  uint32_t adcRaw = analogRead(HB_AN_PIN);
  
  /* Calculate voltage at ADC pin */
  uint32_t voltage_mV = (ACD_VOLTAGE_RANGE_MVOLTS * adcRaw) / ACD_RESOLUTION_STEPS;
  
  /* Calculate motor current based on BTN9960LV IS characteristic and voltage divider
     ILOAD [mA] = V_ADC [V] * 7752 = V_ADC [mV] * 7.752 */
  uint32_t current_mA = (voltage_mV * 7752) / 1000;
  
  return (uint16_t)current_mA;
}

/**
 * @brief Play a tone on the buzzer
 * @param note Musical note to play
 * @param ms Duration in milliseconds
 */
void sound(note_t note, int ms) {
  ledcAttachChannel(BUZZ_PIN, 5000, 8, BUZZ_CHAN);
  ledcWriteNote(BUZZ_PIN, note, 7);
  delay(ms);
  ledcDetach(BUZZ_PIN);
}

/**
 * @brief Play power-off sound (E -> C)
 */
void offSound() { 
  sound(NOTE_E, 60);
  delay(60);
  sound(NOTE_C, 60);  
}

/**
 * @brief Play power-on sound (C -> E)
 */
void onSound() { 
  sound(NOTE_C, 30);
  sound(NOTE_E, 30);
}

/**
 * @brief Play calibration mode sound (C -> G -> A)
 */
void calibSound() { 
  sound(NOTE_C, 60);
  delay(60);
  sound(NOTE_G, 60);  
  delay(60);
  sound(NOTE_A, 60);  
}

/**
 * @brief Play key press sound
 */
void keySound() { 
  sound(NOTE_D, KEY_SOUND_MS);
}