#include "settings_power_menu.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_text_access.h"
#include "settings_display_submenus.h"
#include "settings_power_sleep_submenus.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();
extern bool isEscapeToMainRequested();
extern void setLastEncoderInteraction(uint32_t interactionMs);

extern void showScreensaver();
extern void saveEEPROM(StoredVar_type toSave);

/**
 * Power settings submenu: SCRSV, SLEEP, DEEP SLEEP, STARTUP DELAY, BACK.
 * SCRSV/SLEEP/DEEP SLEEP open submenus; STARTUP DELAY is edited inline.
 */
void showPowerSettings() {
  uint16_t lang = g_storedVar.language;
  const uint8_t NUM = POWER_ITEMS_COUNT;  /* 5: SCRSV, SLEEP, DEEP SLEEP, STARTUP, BACK */
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH    = HEIGHT8x8;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
  g_rotaryEncoder.reset(0);

  uint8_t sel = 0;
  bool editing = false;
  uint16_t tmpDelay = g_storedVar.startupDelay;
  uint16_t origDelay = g_storedVar.startupDelay;
  bool needRedraw = true;
  uint32_t lastInteraction = millis();
  bool ssActive = false;
  uint16_t ssEncoderPos = 0;

  auto resumeAfterChildMenu = [&](uint8_t selectedIndex) {
    lastInteraction = millis();
    setLastEncoderInteraction(lastInteraction);
    ssActive = false;
    ssEncoderPos = g_rotaryEncoder.readEncoder();
    g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
    g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
    g_rotaryEncoder.reset(selectedIndex);
    obdFill(&g_obd, OBD_WHITE, 1);
    needRedraw = true;
  };

  while (true) {
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;

    /* Screensaver wake-up */
    if (ssActive) {
      uint16_t curPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          curPos != ssEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        ssActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        needRedraw = true;
      }
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    /* Screensaver timeout */
    if (!wakeUp && !ssActive && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!ssActive) {
          ssActive = true;
          ssEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &ssActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
          needRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    /* Encoder movement */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (editing) {
        tmpDelay = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (uint8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    /* Encoder button click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        /* Confirm startup delay */
        g_storedVar.startupDelay = tmpDelay;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
        g_rotaryEncoder.reset(3);
        needRedraw = true;
      } else if (sel == 0) {  /* SCRSV */
        showScreensaverSettings();
        if (isEscapeToMainRequested()) break;
        resumeAfterChildMenu(0);
      } else if (sel == 1) {  /* SLEEP */
        showSleepSettings();
        if (isEscapeToMainRequested()) break;
        resumeAfterChildMenu(1);
      } else if (sel == 2) {  /* DEEP SLEEP */
        showDeepSleepSettings();
        if (isEscapeToMainRequested()) break;
        resumeAfterChildMenu(2);
      } else if (sel == 3) {  /* STARTUP DELAY - enter edit */
        editing = true;
        origDelay = g_storedVar.startupDelay;
        tmpDelay  = g_storedVar.startupDelay;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(STARTUP_DELAY_MIN, STARTUP_DELAY_MAX, false);
        g_rotaryEncoder.reset(tmpDelay);
        needRedraw = true;
      } else {  /* BACK */
        break;
      }
    }

    /* Brake button = cancel edit or back */
    static bool brakeBtnInPower = false;
    static uint32_t lastBrakeBtnPowerTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInPower && millis() - lastBrakeBtnPowerTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInPower = true;
        lastBrakeBtnPowerTime = millis();
        if (editing) {
          g_storedVar.startupDelay = origDelay;
          tmpDelay = origDelay;
          editing = false;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
          g_rotaryEncoder.reset(3);
          needRedraw = true;
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeBtnInPower = false;
    }

    /* Draw menu */
    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);
      for (uint8_t i = 0; i < NUM; i++) {
        bool isHighlighted = editing ? (i == 3) : (sel == i);
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)getPowerMenuName(lang, i),
                       menuFont, isHighlighted ? OBD_WHITE : OBD_BLACK, 1);
        /* Show startup delay value on the right for item 3 */
        if (i == 3) {
          char valStr[7];
          uint16_t disp = editing ? tmpDelay : g_storedVar.startupDelay;
          if (disp == 0) snprintf(valStr, sizeof(valStr), "   %s", getOnOffLabel(lang, 0));
          else           snprintf(valStr, sizeof(valStr), "%3dms", disp * 10);
          uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * WIDTH8x8);
          obdWriteString(&g_obd, 0, vx, i * lineH, valStr, menuFont,
                         isHighlighted ? OBD_WHITE : OBD_BLACK, 1);
        }
      }
      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }
}
