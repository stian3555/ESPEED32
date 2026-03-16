#include "task_control_loop.h"
#include <Arduino.h>
#include "HAL.h"
#include "slot_ESC.h"

extern StateMachine_enum g_currState;
extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern uint16_t g_carSel;
extern uint32_t g_lastEncoderInteraction;

extern uint16_t normalizeAndClamp(uint16_t raw, uint16_t minIn, uint16_t maxIn, uint16_t normalizedMax, bool isReversed);
extern uint16_t addDeadBand(uint16_t inputVal, uint16_t minVal, uint16_t maxVal, uint16_t deadBand);
extern uint16_t throttleCurve2(uint16_t inputThrottleNorm);
extern uint16_t throttleAntiSpin3(uint16_t requestedSpeed);

void Task2code(void *pvParameters) {
  static unsigned long prevCallTime_uS = 0;
  static unsigned long currTrigger_raw = 0, prevTrigger_raw = 0;

  HalfBridge_Enable();

  for (;;) {
    unsigned long currCallTime_uS = micros();
    unsigned long deltaTime_uS = currCallTime_uS - prevCallTime_uS;
    
    /* Execute every ESC_PERIOD_US microseconds */
    if (deltaTime_uS > ESC_PERIOD_US) {
      prevCallTime_uS = currCallTime_uS;

      /* Read and filter trigger input */
      prevTrigger_raw = currTrigger_raw;
      currTrigger_raw = HAL_ReadTriggerRaw();
      g_escVar.trigger_raw = (prevTrigger_raw + currTrigger_raw) / 2;  /* Simple moving average filter */
      
      /* Normalize and apply deadband */
      g_escVar.trigger_norm = normalizeAndClamp(g_escVar.trigger_raw, g_storedVar.minTrigger_raw, 
                                                g_storedVar.maxTrigger_raw, THROTTLE_NORMALIZED, THROTTLE_REV);
      g_escVar.trigger_norm = addDeadBand(g_escVar.trigger_norm, 0, THROTTLE_NORMALIZED, THROTTLE_DEADBAND_NORM);
      
      /* Lap detection requires motor-load sensing.
         Builds without current sense keep lap timing disabled. */
      {
        static uint8_t lapState = 0;  /* 0=TRACKING, 1=GAP, 2=COOLDOWN */
        static uint32_t gapStartMs = 0;
        static uint32_t lapRegisteredMs = 0;
        static uint32_t driveCurrentEma_mA = 0;

        uint32_t vinRaw = analogRead(AN_VIN_DIV);
        uint32_t vinMv = (ACD_VOLTAGE_RANGE_MVOLTS * vinRaw / ACD_RESOLUTION_STEPS)
                         * (RVIFBL + RVIFBH) / RVIFBL;
        bool hasCurrentSense = HAL_HasMotorCurrentSense();
        uint16_t motorCurrent_mA = hasCurrentSense ? HAL_ReadMotorCurrent() : 0;
        g_escVar.motorCurrent_mA = motorCurrent_mA;

        /* Update display voltage with 8-sample moving average to filter ADC noise */
        static uint32_t vinFilter[8] = {0};
        static uint8_t vinFilterIdx = 0;
        vinFilter[vinFilterIdx] = vinMv;
        vinFilterIdx = (vinFilterIdx + 1) % 8;
        uint32_t vinSum = 0;
        for (uint8_t f = 0; f < 8; f++) vinSum += vinFilter[f];
        g_escVar.Vin_mV = (uint16_t)(vinSum / 8);

        uint32_t throttlePct = ((uint32_t)g_escVar.trigger_norm * 100U) / THROTTLE_NORMALIZED;
        bool throttleActive = (throttlePct >= LAP_TRIGGER_ACTIVE_PCT);
        bool inDeadSpot = false;
        bool deadSpotRecovered = false;

        if (hasCurrentSense) {
          if (throttleActive && motorCurrent_mA >= LAP_CURRENT_GAP_MIN_MA) {
            if (driveCurrentEma_mA == 0) driveCurrentEma_mA = motorCurrent_mA;
            else driveCurrentEma_mA = (driveCurrentEma_mA * 7U + motorCurrent_mA) / 8U;
          } else if (driveCurrentEma_mA > 0) {
            driveCurrentEma_mA = (driveCurrentEma_mA * 7U) / 8U;
          }

          uint32_t currentGapThreshold = driveCurrentEma_mA / 6U;      /* ~17% of running baseline */
          uint32_t currentRecoverThreshold = driveCurrentEma_mA / 3U;  /* ~33% of running baseline */
          if (currentGapThreshold < LAP_CURRENT_GAP_MIN_MA) currentGapThreshold = LAP_CURRENT_GAP_MIN_MA;
          if (currentRecoverThreshold < LAP_CURRENT_RECOVER_MIN_MA) currentRecoverThreshold = LAP_CURRENT_RECOVER_MIN_MA;

          bool currentDetectArmed = throttleActive && (driveCurrentEma_mA >= LAP_CURRENT_BASE_MIN_MA);
          inDeadSpot = currentDetectArmed && (motorCurrent_mA <= currentGapThreshold);
          deadSpotRecovered = (motorCurrent_mA >= currentRecoverThreshold);
        } else {
          driveCurrentEma_mA = 0;
          lapState = 0;
        }

        uint32_t nowMs = millis();

        switch (lapState) {
          case 0: /* TRACKING */
            if (inDeadSpot) {
              gapStartMs = nowMs;
              lapState = 1;
            }
            break;
          case 1: /* GAP */
            if (deadSpotRecovered) {
              if ((nowMs - gapStartMs) <= LAP_GAP_MAX_MS) {
                /* Valid dead spot crossing */
                if (g_escVar.lapStartTime_ms > 0) {
                  uint32_t lapTime = nowMs - g_escVar.lapStartTime_ms;
                  uint8_t idx = g_escVar.lapCount % LAP_MAX_COUNT;
                  g_escVar.lapTimes[idx] = lapTime;
                  if (g_escVar.lapCount == 0 || lapTime < g_escVar.bestLapTime_ms)
                    g_escVar.bestLapTime_ms = lapTime;
                  g_escVar.lapCount++;
                }
                g_escVar.lapStartTime_ms = nowMs;
                lapRegisteredMs = nowMs;
                lapState = 2;
              } else {
                lapState = 0; /* Too long = crash/stop */
              }
            } else if ((nowMs - gapStartMs) > LAP_GAP_MAX_MS) {
              lapState = 0; /* Still in gap and too long - abort */
            }
            break;
          case 2: /* COOLDOWN */
            if ((nowMs - lapRegisteredMs) >= LAP_MIN_TIME_MS) {
              lapState = 0;
            }
            break;
        }
      }

      /* Apply motor control (skip if in calibration or init state) */
      if (!(g_currState == CALIBRATION || g_currState == INIT)) {
        if (g_escVar.trigger_norm == 0) {
          /* Apply brake when trigger is released */
          /* Check if brake button is pressed - use alternate brake value */
          uint16_t effectiveBrake = g_storedVar.carParam[g_carSel].brake;
          if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
            /* Use brakeButtonReduction as alternate brake value (not a reduction) */
            effectiveBrake = g_storedVar.carParam[g_carSel].brakeButtonReduction;
          }
          HalfBridge_SetPwmDrag(0, effectiveBrake);
          g_escVar.outputSpeed_pct = 0;
          throttleAntiSpin3(0);  /* Keep anti-spin timer updated */
        } else {
          /* Check if quick brake zone is active */
          bool inQuickBrakeZone = false;
          if (g_storedVar.carParam[g_carSel].quickBrakeEnabled) {
            uint16_t qbThreshold_norm = (uint32_t)g_storedVar.carParam[g_carSel].quickBrakeThreshold
                                        * THROTTLE_NORMALIZED / 100;
            inQuickBrakeZone = (g_escVar.trigger_norm < qbThreshold_norm);
          }
          if (inQuickBrakeZone) {
            /* Quick brake zone: apply configured brake strength */
            HalfBridge_SetPwmDrag(0, g_storedVar.carParam[g_carSel].quickBrakeStrength);
            g_escVar.outputSpeed_pct = 0;
            throttleAntiSpin3(0);  /* Keep anti-spin timer updated */
          } else {
            /* Apply throttle curve and anti-spin */
            g_escVar.outputSpeed_pct = throttleCurve2(g_escVar.trigger_norm);
            g_escVar.outputSpeed_pct = throttleAntiSpin3(g_escVar.outputSpeed_pct);
            HalfBridge_SetPwmDrag(g_escVar.outputSpeed_pct, 0);
          }
          /* Update last interaction time to prevent screensaver while driving */
          g_lastEncoderInteraction = millis();
        }
      }
    }
  }
}
