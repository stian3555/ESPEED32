#include "settings_quick_brake_menu.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_text_access.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern uint16_t g_carSel;
extern AiEsp32RotaryEncoder g_rotaryEncoder;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();

extern void showScreensaver();
extern void saveEEPROM(StoredVar_type toSave);
extern void resetEncoderForMainMenu();

/**
 * Quick Brake submenu.
 * Opened by clicking the QB item in the main menu.
 * Items: QB (ON/OFF), QB_TH (threshold %), QB_ST (strength %), BACK.
 * The main menu still shows ON/OFF for the QB item at a glance.
 */
void showQuickBrakeMenu() {
  const uint8_t QB_ITEMS = 4;  /* QB, QB_TH, QB_ST, BACK */
  uint8_t lang = g_storedVar.language;

  const char* rowNames[QB_ITEMS];
  if (lang == LANG_NOR) {
    rowNames[0] = "Aktiv";
    rowNames[1] = "Terskel";
    rowNames[2] = "Styrke";
    rowNames[3] = "Tilbake";
  } else {
    rowNames[0] = "Enable";
    rowNames[1] = "Threshold";
    rowNames[2] = "Strength";
    rowNames[3] = "Back";
  }

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
  g_rotaryEncoder.reset(1);

  uint8_t sel          = 1;
  uint8_t prevSel      = 0xFF;
  MenuState_enum state = ITEM_SELECTION;
  uint16_t origValue   = 0;
  bool forceRedraw     = true;

  uint32_t lastInteraction   = millis();
  bool screensaverActive     = false;
  uint16_t screensaverEncPos = 0;

  static bool brakeInQB = false;
  static uint32_t lastBrakeQB = 0;

  while (true) {
    /* Screensaver */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          ep != screensaverEncPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }
    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel     = 0xFF;
          forceRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    /* Brake button: cancel edit or exit submenu */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeInQB && millis() - lastBrakeQB > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeInQB = true;
        lastBrakeQB = millis();
        if (state == VALUE_SELECTION) {
          /* Cancel: restore original value */
          if (sel == 1) g_storedVar.carParam[g_carSel].quickBrakeEnabled = origValue;
          else if (sel == 2) g_storedVar.carParam[g_carSel].quickBrakeThreshold = origValue;
          else if (sel == 3) g_storedVar.carParam[g_carSel].quickBrakeStrength = origValue;
          state = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
          g_rotaryEncoder.reset(sel);
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel     = 0xFF;
          forceRedraw = true;
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeInQB = false;
    }

    /* Encoder scroll */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      uint16_t ep = (uint16_t)g_rotaryEncoder.readEncoder();
      if (state == ITEM_SELECTION) {
        sel = (uint8_t)ep;
        forceRedraw = true;
      } else {
        /* Live update while editing */
        if (sel == 1) g_storedVar.carParam[g_carSel].quickBrakeEnabled = ep;
        else if (sel == 2) g_storedVar.carParam[g_carSel].quickBrakeThreshold = ep;
        else if (sel == 3) g_storedVar.carParam[g_carSel].quickBrakeStrength = ep;
        forceRedraw = true;
      }
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (state == ITEM_SELECTION) {
        if (sel == QB_ITEMS) {
          /* BACK */
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
        /* Enter value edit */
        uint16_t minV = 0, maxV = 1, curV = 0;
        if (sel == 1) { curV = g_storedVar.carParam[g_carSel].quickBrakeEnabled; minV = 0; maxV = 1; }
        else if (sel == 2) { curV = g_storedVar.carParam[g_carSel].quickBrakeThreshold; minV = 0; maxV = QUICK_BRAKE_THRESHOLD_MAX; }
        else if (sel == 3) { curV = g_storedVar.carParam[g_carSel].quickBrakeStrength; minV = 0; maxV = QUICK_BRAKE_STRENGTH_MAX; }
        origValue = curV;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(minV, maxV, false);
        g_rotaryEncoder.reset(curV);
        state       = VALUE_SELECTION;
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      } else {
        /* Confirm value */
        saveEEPROM(g_storedVar);
        state = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }

    /* Draw */
    if (forceRedraw || sel != prevSel) {
      forceRedraw = false;
      prevSel = sel;

      for (uint8_t idx = 0; idx < QB_ITEMS; idx++) {
        uint8_t yPx     = idx * HEIGHT8x8;
        bool isSelected = (sel == idx + 1);
        bool inEdit     = (state == VALUE_SELECTION && isSelected);

        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowNames[idx], FONT_8x8,
                       (isSelected && state == ITEM_SELECTION) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value right-justified */
        char vbuf[8];
        if (idx == 0) {
          uint16_t v = g_storedVar.carParam[g_carSel].quickBrakeEnabled;
          snprintf(vbuf, sizeof(vbuf), "%3s", getOnOffLabel(lang, v ? 1 : 0));
        } else if (idx == 1) {
          snprintf(vbuf, sizeof(vbuf), "%3d%%", g_storedVar.carParam[g_carSel].quickBrakeThreshold);
        } else if (idx == 2) {
          snprintf(vbuf, sizeof(vbuf), "%3d%%", g_storedVar.carParam[g_carSel].quickBrakeStrength);
        } else {
          vbuf[0] = '\0';
        }
        if (vbuf[0]) {
          int tw = strlen(vbuf) * WIDTH8x8;
          obdWriteString(&g_obd, 0, OLED_WIDTH - tw, yPx, vbuf, FONT_8x8,
                         inEdit ? OBD_WHITE : OBD_BLACK, 1);
        }
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
  resetEncoderForMainMenu();
}
