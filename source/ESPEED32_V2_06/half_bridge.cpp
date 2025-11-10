/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include "half_bridge.h"

using namespace btn99x0;

/*********************************************************************************************************************/
/*                                                Private Variables                                                  */
/*********************************************************************************************************************/

/* Half-bridge chip variant */
ic_variant_t icVariant = IC_VARIANT_BTN9990LV;

/* Pin assignments for half-bridge signals */
io_pins_t ioPins = {
  .analog = HB_AN_PIN,
  .input = HB_IN_PIN,
  .inhibit = HB_INH_PIN
};

/* Hardware configuration for ADC diagnostic */
hw_conf_t hwConf = {
  .sense_current_resistor_ohms = 2000,
  .adc_voltage_range_volts = ACD_VOLTAGE_RANGE_MVOLTS / 1000.0,
  .adc_resolution_steps = ACD_RESOLUTION_STEPS
};

/* Half-bridge driver instance */
HalfBridge halfBridge(icVariant, ioPins, hwConf);

/*********************************************************************************************************************/
/*                                            Function Implementations                                              */
/*********************************************************************************************************************/

/**
 * @brief Initialize half-bridge driver
 * @details Sets slew rate and differential current sense ratio
 */
void HalfBridge_Setup() {
  halfBridge.set_slew_rate(SLEW_RATE_LEVEL_7);
  halfBridge.set_dk(50000);  /* Experimentally measured value for this setup */
  
  HAL_InitHW();
}

/**
 * @brief Set PWM duty cycle with drag brake
 * @param duty_pct Motor duty cycle [0-100%]
 * @param drag_pct Drag brake percentage [0-100%]
 */
void HalfBridge_SetPwmDrag(uint8_t duty_pct, uint8_t drag_pct) {
  halfBridge.set_pwm_drag(duty_pct, drag_pct);
}

/**
 * @brief Enable half-bridge output
 */
void HalfBridge_Enable() {
  halfBridge.enable();
}

/**
 * @brief Test motor with various PWM patterns
 * @details Full speed -> coast -> brake sequence
 */
void HalfBridge_TestMotor() {
  halfBridge.set_pwm_drag(100, 0);  /* Full power */
  delay(300);
  
  halfBridge.set_pwm_drag(0, 0);    /* Coast */
  delay(1000);
  
  halfBridge.set_pwm_drag(50, 100); /* 50% duty with full brake */
  delay(300);
  
  halfBridge.set_pwm_drag(0, 100);  /* Full brake */
  delay(1000);
}