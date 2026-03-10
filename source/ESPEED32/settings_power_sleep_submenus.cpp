#include "settings_power_sleep_submenus.h"
#include <Arduino.h>
#include "slot_ESC.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();

extern void showScreensaver();
extern void showPowerSave();
extern void showDeepSleep();
extern void saveEEPROM(StoredVar_type toSave);

/**
 * Sleep settings submenu: NOW (manual sleep) and INTERVAL (auto-sleep timeout).
 */
void showSleepSettings() {
  uint16_t lang = g_storedVar.language;

  /* Labels [NOR, ENG, CS, ACD] */
  const char* lblInterval[4] = {"INTERV.",  "INTERVAL", "INTERVAL", "INTERVAL"};
  const char* lblNow[4]      = {"SOV NA",   "SLEEP NOW", "SLEEP NOW", "SLEEP NOW"};
  const char* lblBack[4]     = {"TILBAKE",  "BACK",      "BACK",      "BACK"};
  const char* lblOff[4]      = {"AV",       "OFF",       "OFF",       "OFF"};

  const uint8_t ITEM_NOW = 0;
  const uint8_t ITEM_INTERVAL = 1;
  const uint8_t ITEM_BACK = 2;
  const uint8_t NUM_ITEMS = 3;  /* 0=NOW, 1=INTERVAL, 2=BACK */
  const uint8_t menuFont  = FONT_8x8;
  const uint8_t charWidth = WIDTH8x8;
  const uint8_t lineH     = HEIGHT8x8;

  int8_t sel = 0;
  bool editing = false;
  uint16_t tmpTimeout = g_storedVar.powerSaveTimeout;
  bool needRedraw = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

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
        obdFill(&g_obd, OBD_WHITE, 1);
        needRedraw = true;
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
          needRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        obdFill(&g_obd, OBD_WHITE, 1);
        needRedraw = true;
      }
      delay(10);
      continue;
    }

    /* Encoder movement */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (editing) {
        tmpTimeout = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (int8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    /* Encoder button */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        /* Confirm new interval */
        g_storedVar.powerSaveTimeout = tmpTimeout;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(ITEM_INTERVAL);
      } else if (sel == ITEM_INTERVAL) {
        /* Enter interval editing */
        editing = true;
        tmpTimeout = g_storedVar.powerSaveTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, POWER_SAVE_TIMEOUT_MAX, false);
        g_rotaryEncoder.reset(tmpTimeout);
      } else if (sel == ITEM_NOW) {
        /* Sleep NOW */
        showPowerSave();
        lastInteraction = millis();
        screensaverActive = false;
        needRedraw = true;
      } else if (sel == ITEM_BACK) {
        /* BACK */
        break;
      }
      needRedraw = true;
    }

    /* Brake button = cancel/back */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (editing) {
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(ITEM_INTERVAL);
        needRedraw = true;
        delay(200);
      } else {
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    }

    /* Draw */
    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);

      /* Row 0: NOW */
      bool s0 = (!editing && sel == ITEM_NOW);
      obdWriteString(&g_obd, 0, 0, 0, (char*)lblNow[lang], menuFont, s0 ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 1: INTERVAL + value */
      bool s1 = (!editing && sel == ITEM_INTERVAL);
      obdWriteString(&g_obd, 0, 0, lineH, (char*)lblInterval[lang], menuFont, s1 ? OBD_WHITE : OBD_BLACK, 1);
      char valStr[8];
      uint16_t disp = editing ? tmpTimeout : g_storedVar.powerSaveTimeout;
      if (disp == 0) snprintf(valStr, sizeof(valStr), "  %s", lblOff[lang]);
      else           snprintf(valStr, sizeof(valStr), "%2dmin", disp);
      uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * charWidth);
      obdWriteString(&g_obd, 0, vx, lineH, valStr, menuFont, (s1 || editing) ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 2: BACK */
      bool s2 = (!editing && sel == ITEM_BACK);
      obdWriteString(&g_obd, 0, 0, 2 * lineH, (char*)lblBack[lang], menuFont, s2 ? OBD_WHITE : OBD_BLACK, 1);

      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }
}

/**
 * Deep sleep settings submenu: NOW (manual deep sleep) and INTERVAL (auto deep-sleep timeout).
 * Uses hardcoded FONT_8x8 regardless of listFontSize.
 */
void showDeepSleepSettings() {
  uint16_t lang = g_storedVar.language;

  const char* lblInterval[4] = {"INTERV.",    "INTERVAL",   "INTERVAL",   "INTERVAL"};
  const char* lblNow[4]      = {"SLUKK NA",   "SLEEP NOW",  "SLEEP NOW",  "SLEEP NOW"};
  const char* lblBack[4]     = {"TILBAKE",    "BACK",       "BACK",       "BACK"};
  const char* lblOff[4]      = {"AV",         "OFF",        "OFF",        "OFF"};

  const uint8_t ITEM_NOW = 0;
  const uint8_t ITEM_INTERVAL = 1;
  const uint8_t ITEM_BACK = 2;
  const uint8_t NUM_ITEMS = 3;  /* 0=NOW, 1=INTERVAL, 2=BACK */

  int8_t sel = 0;
  bool editing = false;
  uint16_t tmpTimeout = g_storedVar.deepSleepTimeout;
  bool needRedraw = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

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
        obdFill(&g_obd, OBD_WHITE, 1);
        needRedraw = true;
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
          needRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        obdFill(&g_obd, OBD_WHITE, 1);
        needRedraw = true;
      }
      delay(10);
      continue;
    }

    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (editing) {
        tmpTimeout = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (int8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        g_storedVar.deepSleepTimeout = tmpTimeout;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(ITEM_INTERVAL);
      } else if (sel == ITEM_INTERVAL) {
        editing = true;
        tmpTimeout = g_storedVar.deepSleepTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        /* 0=OFF, DEEP_SLEEP_TIMEOUT_MIN-DEEP_SLEEP_TIMEOUT_MAX; encode 0 as OFF */
        g_rotaryEncoder.setBoundaries(0, DEEP_SLEEP_TIMEOUT_MAX, false);
        g_rotaryEncoder.reset(tmpTimeout);
      } else if (sel == ITEM_NOW) {
        /* Deep sleep NOW - never returns */
        showDeepSleep();
        needRedraw = true;
      } else if (sel == ITEM_BACK) {
        break;
      }
      needRedraw = true;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (editing) {
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(ITEM_INTERVAL);
        needRedraw = true;
        delay(200);
      } else {
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    }

    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);

      /* Row 0: NOW */
      bool s0 = (!editing && sel == ITEM_NOW);
      obdWriteString(&g_obd, 0, 0, 0, (char*)lblNow[lang], FONT_8x8, s0 ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 1: INTERVAL + value */
      bool s1 = (!editing && sel == ITEM_INTERVAL);
      obdWriteString(&g_obd, 0, 0, HEIGHT8x8, (char*)lblInterval[lang], FONT_8x8, s1 ? OBD_WHITE : OBD_BLACK, 1);
      char valStr[8];
      uint16_t disp = editing ? tmpTimeout : g_storedVar.deepSleepTimeout;
      if (disp == 0) snprintf(valStr, sizeof(valStr), "  %s", lblOff[lang]);
      else           snprintf(valStr, sizeof(valStr), "%2dmin", disp);
      uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * WIDTH8x8);
      obdWriteString(&g_obd, 0, vx, HEIGHT8x8, valStr, FONT_8x8, (s1 || editing) ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 2: BACK */
      bool s2 = (!editing && sel == ITEM_BACK);
      obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, (char*)lblBack[lang], FONT_8x8, s2 ? OBD_WHITE : OBD_BLACK, 1);

      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }
}
