#ifndef HAL_H_
#define HAL_H_

/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include <Wire.h>
#include <Arduino.h>

/*********************************************************************************************************************/
/*                                             Hardware Definitions                                                 */
/*********************************************************************************************************************/

/* Display Configuration */
#define MY_OLED     OLED_128x64
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define USE_BACKBUFFER  /* Comment out if not enough RAM for back buffer */

/* Voltage Divider and Shunt Resistors */
#define RVIFBL 2200UL    /* [Ohm] VIN ADC resistor divider, lower resistor */
#define RVIFBH 10000UL   /* [Ohm] VIN ADC resistor divider, upper resistor */

/* ADC Configuration */
#define THROTTLE_NORMALIZED         256
#define THROTTLE_DEADBAND_PERC      3   /* [%] Throttle deadband percentage */
#define THROTTLE_DEADBAND_NORM      ((THROTTLE_DEADBAND_PERC * THROTTLE_NORMALIZED) / 100)
#define THROTTLE_NOISE_PERC         2
#define THROTTLE_NOISE_NORM         ((THROTTLE_NOISE_PERC * THROTTLE_NORMALIZED) / 100)
#define ACD_RESOLUTION_STEPS        4095

/* ADC Voltage Calibration */
#define VIN_CAL_SET  1200
#define VIN_CAL_READ 1108
#define ACD_VOLTAGE_RANGE_MVOLTS  ((3300 * VIN_CAL_SET) / VIN_CAL_READ)  /* Calibrated ADC voltage range */

#define MAX_INT16   32767   /* Maximum int16 value for calibration */
#define MIN_INT16   -32768  /* Minimum int16 value for calibration */

/* PWM Configuration */
#define THR_IN_PWM_CHAN   0     /* PWM channel for motor input */
#define THR_INH_PWM_CHAN  1     /* PWM channel for motor inhibit */
#define BUZZ_CHAN         6     /* PWM channel for buzzer */
#define THR_PWM_RES_BIT   8     /* PWM resolution in bits */ 

/* Trigger Sensor Selection */
/* Uncomment ONLY ONE of the following sensor types */
//#define AS5600_MAG      /* AS5600 magnetic sensor (default) */
#define TLE493D_MAG     /* Infineon TLE493D magnetic sensor */
//#define AS5600L_MAG     /* AS5600L variant with different I2C address */
//#define ANALOG_TRIG     /* Analog potentiometer trigger */
//#define MT6701_MAG      /* MT6701 magnetic sensor */

/* Trigger Reversal Configuration */
#if defined(AS5600_MAG) || defined(AS5600L)
  #define THROTTLE_REV  1  /* 1 = trigger inverted (full press = minimum ADC value) */
#elif defined(MT6701_MAG)
  #define THROTTLE_REV  1
#elif defined(ANALOG_TRIG)
  #define THROTTLE_REV  1
#elif defined(TLE493D_MAG)
  #define THROTTLE_REV  0  /* 0 = trigger normal (full press = maximum ADC value) */
#endif

/* Miscellaneous */
#define KEY_SOUND_MS      50
#define BUTTON_PRESSED    0

/*********************************************************************************************************************/
/*                                                 Pin Definitions                                                   */
/*********************************************************************************************************************/

/* I2C Pins */
#define SDA0_PIN    21    /* I2C #1 SDA: Magnetic sensor */
#define SCL0_PIN    22    /* I2C #1 SCL: Magnetic sensor */
#define SDA1_PIN    33    /* I2C #2 SDA: OLED display */
#define SCL1_PIN    32    /* I2C #2 SCL: OLED display */

/* OLED Display Configuration */
#define RESET_PIN   -1    /* Set to -1 to disable external reset */
#define OLED_ADDR   -1    /* Let OneBitDisplay auto-detect address */
#define FLIP180     0     /* Don't rotate display 180° */
#define INVERT_DISP 0     /* Don't invert display colors */
#define USE_HW_I2C  1     /* Use hardware I2C (not bit-bang) */

/* Analog Input Pins */
#define AN_VIN_DIV  36    /* Voltage divider input */

/* Rotary Encoder Pins */
#define ENCODER_A_PIN      16  /* Encoder signal A (S1) */
#define ENCODER_B_PIN      17  /* Encoder signal B (S2) */
#define ENCODER_BUTTON_PIN 4   /* Encoder button (KEY) */
#define ENCODER_VCC_PIN    -1  /* Set to -1 if encoder VCC connected directly to 3.3V */
#define ENCODER_STEPS      4

/* Motor Control Pins */
#define AN_MOT_BEMF  14   /* Motor back-EMF sensing */
#define HB_AN_PIN    25   /* Half-bridge analog feedback */
#define HB_IN_PIN    26   /* Half-bridge IN pin */
#define HB_INH_PIN   27   /* Half-bridge INH pin */
#define LED_BUILTIN  2    /* Built-in LED */

/* Other Pins */
#define BUTT_PIN   13   /* Button */
#define BUZZ_PIN   18   /* Buzzer */

/*********************************************************************************************************************/
/*                                              Function Prototypes                                                 */
/*********************************************************************************************************************/
void     HAL_InitHW();
uint16_t HAL_ReadVoltageDivider(int analogInput, uint32_t rvfbl, uint32_t rvfbh);
int16_t  HAL_ReadTriggerRaw();
void     HALanalogWrite(int pwmChan, int value);
void     HAL_PinSetup();

/* Sound Functions */
void sound(note_t note, int ms);
void offSound();
void onSound();
void calibSound();
void keySound();

#endif  /* HAL_H_ */