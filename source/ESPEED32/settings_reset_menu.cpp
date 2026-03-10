#include "settings_reset_menu.h"
#include <Arduino.h>
#include "slot_ESC.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern Preferences g_pref;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();

extern void showScreensaver();
extern void saveEEPROM(StoredVar_type toSave);
extern void initMenuItems();
extern void initSettingsMenuItems();

/**
 * Show a double-click confirmation for a destructive reset action.
 * @param label  Short label shown on the confirm screen (e.g. "CAR")
 * @return true if user confirmed twice, false if cancelled
 */
static bool resetConfirm(const char* label) {
  uint16_t lang = g_storedVar.language;
  const char* cancelText = (lang == LANG_NOR) ? "TILBAKE=AVBRYT" : "BACK=CANCEL";

  auto showCancelled = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    const char* msg = (lang == LANG_NOR) ? "AVBRUTT!" : "CANCELLED!";
    int tw = strlen(msg) * WIDTH12x16;
    obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 24, (char *)msg, FONT_12x16, OBD_BLACK, 1);
    delay(1000);
    obdFill(&g_obd, OBD_WHITE, 1);
  };

  const char* pressT = (lang == LANG_NOR) ? "TRYKK 2 GANGER" : "PRESS TWICE";
  auto drawFirstConfirm = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 10, 5,  (char *)label, FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 10, 25, (char *)pressT,  FONT_6x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 10, 45, (char *)cancelText, FONT_6x8, OBD_BLACK, 1);
  };

  auto drawSecondConfirm = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    if (lang == LANG_NOR) {
      obdWriteString(&g_obd, 0, 10, 5,  (char *)"TRYKK IGJEN",      FONT_8x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 16, (char *)"FOR A NULLSTILLE", FONT_6x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 24, (char *)label,              FONT_6x8, OBD_BLACK, 1);
    } else {
      obdWriteString(&g_obd, 0, 10, 5,  (char *)"PRESS AGAIN", FONT_8x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 16, (char *)"TO RESET",    FONT_6x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 24, (char *)label,         FONT_6x8, OBD_BLACK, 1);
    }
    obdWriteString(&g_obd, 0, 10, 45, (char *)cancelText, FONT_6x8, OBD_BLACK, 1);
  };

  /* First confirmation */
  drawFirstConfirm();
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  while (true) {
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;

    if (screensaverActive) {
      uint16_t curPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          curPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        drawFirstConfirm();
      }
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          drawFirstConfirm();
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        drawFirstConfirm();
      }
      delay(10);
      continue;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      showCancelled();
      return false;
    }
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
    }
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }
    delay(10);
  }
  delay(200);

  /* Second confirmation - specific label */
  drawSecondConfirm();
  lastInteraction = millis();
  screensaverActive = false;
  screensaverEncoderPos = 0;

  while (true) {
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;

    if (screensaverActive) {
      uint16_t curPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          curPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        drawSecondConfirm();
      }
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          drawSecondConfirm();
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        drawSecondConfirm();
      }
      delay(10);
      continue;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      showCancelled();
      return false;
    }
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
    }
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }
    delay(10);
  }
  delay(200);
  return true;
}

/** Reset all car profiles to factory defaults. */
static void doResetCar() {
  for (uint8_t i = 0; i < CAR_MAX_COUNT; i++) {
    g_storedVar.carParam[i].minSpeed   = MIN_SPEED_DEFAULT;
    g_storedVar.carParam[i].brake      = BRAKE_DEFAULT;
    g_storedVar.carParam[i].antiSpin   = ANTISPIN_DEFAULT;
    g_storedVar.carParam[i].maxSpeed   = MAX_SPEED_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex.inputThrottle = THROTTLE_CURVE_INPUT_THROTTLE_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex.curveSpeedDiff = THROTTLE_CURVE_SPEED_DIFF_DEFAULT;
    g_storedVar.carParam[i].freqPWM              = PWM_FREQ_DEFAULT;
    g_storedVar.carParam[i].brakeButtonReduction = BRAKE_BUTTON_REDUCTION_DEFAULT;
    g_storedVar.carParam[i].quickBrakeEnabled    = QUICK_BRAKE_ENABLED_DEFAULT;
    g_storedVar.carParam[i].quickBrakeThreshold  = QUICK_BRAKE_THRESHOLD_DEFAULT;
    g_storedVar.carParam[i].quickBrakeStrength   = QUICK_BRAKE_STRENGTH_DEFAULT;
    sprintf(g_storedVar.carParam[i].carName, "CAR%d", i);
  }
}

/** Reset all settings (non-car, non-calibration) to factory defaults. */
static void doResetSettings() {
  g_storedVar.viewMode             = VIEW_MODE_LIST;
  g_storedVar.screensaverTimeout   = SCREENSAVER_TIMEOUT_DEFAULT;
  g_storedVar.powerSaveTimeout     = POWER_SAVE_TIMEOUT_DEFAULT;
  g_storedVar.deepSleepTimeout     = DEEP_SLEEP_TIMEOUT_DEFAULT;
  g_storedVar.soundBoot            = SOUND_BOOT_DEFAULT;
  g_storedVar.soundRace            = SOUND_RACE_DEFAULT;
  g_storedVar.gridCarSelectEnabled = GRID_CAR_SELECT_DEFAULT;
  g_storedVar.raceViewMode         = RACE_VIEW_DEFAULT;
  g_storedVar.language             = LANG_DEFAULT;
  g_storedVar.textCase             = TEXT_CASE_DEFAULT;
  g_storedVar.listFontSize         = FONT_SIZE_DEFAULT;
  g_storedVar.startupDelay         = STARTUP_DELAY_DEFAULT;
  strncpy(g_storedVar.screensaverLine1, SCREENSAVER_LINE1_DEFAULT, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine1[SCREENSAVER_TEXT_MAX - 1] = '\0';
  strncpy(g_storedVar.screensaverLine2, SCREENSAVER_LINE2_DEFAULT, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine2[SCREENSAVER_TEXT_MAX - 1] = '\0';
}

/** Reset trigger calibration to factory defaults. */
static void doResetCalibration() {
  g_storedVar.minTrigger_raw = 0;
#if defined(TLE493D_MAG)
  g_storedVar.maxTrigger_raw = 3600;
#else
  g_storedVar.maxTrigger_raw = ACD_RESOLUTION_STEPS;
#endif
}

bool showResetConfirmDialog(const char* label) {
  return resetConfirm(label);
}

void resetAllCarsToFactoryDefaults() {
  doResetCar();
}

/**
 * Reset submenu. Items: CAR, SETTINGS, CALIBRATION, ALL, BACK.
 * Each destructive action requires double-click confirmation.
 */
void showResetSubmenu() {
  const uint8_t RS_ITEMS = 5;
  uint16_t lang = g_storedVar.language;

  const char* rowNames[RS_ITEMS];
  if (lang == LANG_NOR) {
    rowNames[0] = "Bil";
    rowNames[1] = "Innstillinger";
    rowNames[2] = "Kalibrering";
    rowNames[3] = "Alt";
    rowNames[4] = "Tilbake";
  } else {
    rowNames[0] = "Car";
    rowNames[1] = "Settings";
    rowNames[2] = "Calibration";
    rowNames[3] = "Everything";
    rowNames[4] = "Back";
  }

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, RS_ITEMS, false);
  g_rotaryEncoder.reset(1);

  uint8_t sel      = 1;
  uint8_t prevSel  = 0xFF;
  bool forceRedraw = true;
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  static bool brakeInReset = false;
  static uint32_t lastBrakeReset = 0;

  while (true) {
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;

    if (screensaverActive) {
      uint16_t curPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          curPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        forceRedraw = true;
      }
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
          forceRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        obdFill(&g_obd, OBD_WHITE, 1);
        forceRedraw = true;
      }
      delay(10);
      continue;
    }

    /* Brake exits */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (!brakeInReset && millis() - lastBrakeReset > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeInReset = true;
        lastBrakeReset = millis();
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    } else {
      brakeInReset = false;
    }

    /* Encoder scroll */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      sel = (uint8_t)g_rotaryEncoder.readEncoder();
      forceRedraw = true;
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == RS_ITEMS) break;  /* BACK */

      bool confirmed = resetConfirm(rowNames[sel - 1]);
      lastInteraction = millis();
      screensaverActive = false;
      if (confirmed) {
        obdFill(&g_obd, OBD_WHITE, 1);
        const char* doneText = (lang == LANG_NOR) ? "NULLSTILT!" : "RESET DONE!";
        int tw = strlen(doneText) * WIDTH12x16;
        obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 24, (char *)doneText, FONT_12x16, OBD_BLACK, 1);
        delay(1500);

        if (sel == 4) {
          /* Full factory reset: wipe flash, reboot into first-boot path */
          g_pref.begin("stored_var", false);
          g_pref.clear();
          g_pref.end();
          ESP.restart();
        }

        if (sel == 1) { doResetCar(); }
        else if (sel == 2) { doResetSettings(); }
        else if (sel == 3) { doResetCalibration(); }

        saveEEPROM(g_storedVar);
        initMenuItems();
        initSettingsMenuItems();

        if (sel == 3) {
          g_pref.begin("stored_var", false);
          g_pref.putBool("force_calib", true);
          g_pref.end();
          ESP.restart();
        }
      }

      /* Re-init encoder and redraw submenu */
      g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
      g_rotaryEncoder.setBoundaries(1, RS_ITEMS, false);
      g_rotaryEncoder.reset(sel);
      obdFill(&g_obd, OBD_WHITE, 1);
      prevSel     = 0xFF;
      forceRedraw = true;
    }

    /* Draw */
    if (forceRedraw || sel != prevSel) {
      forceRedraw = false;
      prevSel = sel;
      for (uint8_t idx = 0; idx < RS_ITEMS; idx++) {
        uint8_t yPx     = idx * HEIGHT8x8;
        bool isSelected = (sel == idx + 1);
        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowNames[idx], FONT_8x8,
                       isSelected ? OBD_WHITE : OBD_BLACK, 1);
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
