#include "settings_ext_pot_menu.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_text_access.h"
#include "connectivity_portal.h"
#include "ext_pot.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern char msgStr[50];

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();
extern void showScreensaver();
extern void saveEEPROM(StoredVar_type toSave);

void showExtPotSettings() {
  const uint8_t NUM_ITEMS = 3;  /* ACTIVE, TARGET, BACK */
  const uint8_t ITEM_ACTIVE = 0;
  const uint8_t ITEM_TARGET = 1;
  const uint8_t ITEM_BACK = 2;
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH = HEIGHT8x8;

  const char* itemLabels[4][NUM_ITEMS] = {
    {"AKTIV", "STYRER", "TILBAKE"},
    {"ACTIVE", "TARGET", "BACK"},
    {"ACTIVE", "TARGET", "BACK"},
    {"ACTIVE", "TARGET", "BACK"}
  };

  obdFill(&g_obd, OBD_WHITE, 1);

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

  uint8_t sel = 0;
  bool needRedraw = true;

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
      sel = (uint8_t)g_rotaryEncoder.readEncoder();
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == ITEM_BACK) {
        break;
      } else if (sel == ITEM_ACTIVE) {
        g_extPotEnabled = g_extPotEnabled ? 0 : 1;
        resetExtPotFilter();
        saveEEPROM(g_storedVar);
      } else if (sel == ITEM_TARGET) {
        g_extPotTarget = (g_extPotTarget == EXT_POT_TARGET_BRAKE) ? EXT_POT_TARGET_SENSI : EXT_POT_TARGET_BRAKE;
        resetExtPotFilter();
        saveEEPROM(g_storedVar);
      }
      needRedraw = true;
      delay(200);
    }

    static bool brakeBtnInMenu = false;
    static uint32_t lastBrakeBtnTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInMenu && millis() - lastBrakeBtnTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInMenu = true;
        lastBrakeBtnTime = millis();
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    } else {
      brakeBtnInMenu = false;
    }

    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    if (needRedraw) {
      needRedraw = false;
      uint8_t lang = g_storedVar.language;
      for (uint8_t i = 0; i < NUM_ITEMS; i++) {
        bool isSelected = (sel == i);
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)itemLabels[lang][i],
                       menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);

        if (i == ITEM_ACTIVE) {
          snprintf(msgStr, sizeof(msgStr), "%3s", getOnOffLabel(lang, g_extPotEnabled ? 1 : 0));
        } else if (i == ITEM_TARGET) {
          snprintf(msgStr, sizeof(msgStr), "%5s", getRaceLabel(lang, g_extPotTarget == EXT_POT_TARGET_BRAKE ? 0 : 1));
        } else {
          msgStr[0] = '\0';
        }

        if (msgStr[0] != '\0') {
          uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(msgStr) * WIDTH8x8);
          obdWriteString(&g_obd, 0, vx, i * lineH, msgStr,
                         menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
        }
      }
    }

    vTaskDelay(10);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
