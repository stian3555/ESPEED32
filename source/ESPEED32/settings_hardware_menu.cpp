#include "settings_hardware_menu.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_text_access.h"
#include "HAL.h"
#include "settings_ext_pot_menu.h"
#include "diagnostics_self_test.h"

extern StoredVar_type g_storedVar;
extern uint16_t g_encoderInvertEnabled;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern char msgStr[50];

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool refreshIdleInteractionFromControls(uint32_t* lastInteraction, bool* screensaverActive, uint16_t* lastEncoderPos);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();
extern bool isEscapeToMainRequested();
extern void showScreensaver();
extern void saveEEPROM(StoredVar_type toSave);

static const char* HARDWARE_MENU_LABELS[7][HARDWARE_ITEMS_COUNT] = {
  {"ENC INV", "EKST POT", "TRIGGER", "TEST", "TILBAKE"},
  {"ENC INV", "EXT POT", "TRIGGER", "TEST", "BACK"},
  {"ENC INV", "EXT POT", "TRIGGER", "TEST", "BACK"},
  {"ENC INV", "EXT POT", "TRIGGER", "TEST", "BACK"},
  {"ENC INV", "POT EXT", "TRIGGER", "TEST", "ATRAS"},
  {"ENC INV", "EXT POT", "TRIGGER", "TEST", "ZURUCK"},
  {"ENC INV", "POT EST", "TRIGGER", "TEST", "INDIETRO"}
};

static const char* SENSOR_MENU_LABELS[7][4] = {
  {"FAMILIE", "AKTIV", "TYPE", "TILBAKE"},
  {"FAMILY", "ACTIVE", "TYPE", "BACK"},
  {"FAMILY", "ACTIVE", "TYPE", "BACK"},
  {"FAMILY", "ACTIVE", "TYPE", "BACK"},
  {"FAMILIA", "ACTIVO", "TYPE", "ATRAS"},
  {"FAMILIE", "AKTIV", "TYPE", "ZURUCK"},
  {"FAMIGLIA", "ATTIVO", "TYPE", "INDIETRO"}
};

static void showTriggerSensorSettings() {
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH = HEIGHT8x8;
  const bool supportsTypeOverride = HAL_TriggerSensorSupportsTypeOverride();
  const uint8_t numItems = supportsTypeOverride ? 4 : 3;
  const uint8_t itemFamily = 0;
  const uint8_t itemActive = 1;
  const uint8_t itemType = 2;
  const uint8_t itemBack = numItems - 1;

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(0, numItems - 1, false);
  resetUiEncoder(0);

  uint8_t sel = 0;
  bool needRedraw = true;
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = (uint16_t)readUiEncoder();

  while (true) {
    bool wakeUp = refreshIdleInteractionFromControls(&lastInteraction, &screensaverActive, &screensaverEncoderPos);
    if (wakeUp) {
      obdFill(&g_obd, OBD_WHITE, 1);
      needRedraw = true;
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (g_escVar.trigger_norm == 0) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = readUiEncoder();
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
      sel = (uint8_t)readUiEncoder();
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == itemBack) {
        break;
      }
      if (supportsTypeOverride && sel == itemType) {
        uint16_t nextType = HAL_GetTriggerSensorTypeOverride();
        nextType = (nextType >= TRIGGER_SENSOR_TYPE_MAX) ? TRIGGER_SENSOR_TYPE_AUTO : (uint16_t)(nextType + 1);
        (void)HAL_SetTriggerSensorTypeOverride(nextType);
        needRedraw = true;
      }
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
      char familyBuf[16];
      char activeBuf[16];
      char typeBuf[16];
      HAL_GetTriggerSensorFamilyLabel(familyBuf, sizeof(familyBuf));
      HAL_GetTriggerSensorActiveTypeLabel(activeBuf, sizeof(activeBuf));
      HAL_GetTriggerSensorTypeOptionLabel(HAL_GetTriggerSensorTypeOverride(), typeBuf, sizeof(typeBuf));

      for (uint8_t i = 0; i < numItems; i++) {
        bool isSelected = (sel == i);
        const char* label = SENSOR_MENU_LABELS[lang][i];
        if (!supportsTypeOverride && i == itemBack) {
          label = SENSOR_MENU_LABELS[lang][3];
        }
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)label,
                       menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);

        const char* value = nullptr;
        if (i == itemFamily) value = familyBuf;
        else if (i == itemActive) value = activeBuf;
        else if (supportsTypeOverride && i == itemType) value = typeBuf;

        if (value != nullptr && value[0] != '\0') {
          snprintf(msgStr, sizeof(msgStr), "%s", value);
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

void showHardwareSettings() {
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH = HEIGHT8x8;
  const uint8_t itemEncInv = 0;
  const uint8_t itemExtPot = 1;
  const uint8_t itemTrigger = 2;
  const uint8_t itemTest = 3;
  const uint8_t itemBack = 4;

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(0, HARDWARE_ITEMS_COUNT - 1, false);
  resetUiEncoder(0);

  uint8_t sel = 0;
  bool needRedraw = true;
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = (uint16_t)readUiEncoder();

  auto resumeAfterChild = [&]() {
    lastInteraction = millis();
    screensaverActive = false;
    screensaverEncoderPos = (uint16_t)readUiEncoder();
    g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
    setUiEncoderBoundaries(0, HARDWARE_ITEMS_COUNT - 1, false);
    resetUiEncoder(sel);
    obdFill(&g_obd, OBD_WHITE, 1);
    needRedraw = true;
  };

  while (true) {
    bool wakeUp = refreshIdleInteractionFromControls(&lastInteraction, &screensaverActive, &screensaverEncoderPos);
    if (wakeUp) {
      obdFill(&g_obd, OBD_WHITE, 1);
      needRedraw = true;
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (g_escVar.trigger_norm == 0) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = readUiEncoder();
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
      sel = (uint8_t)readUiEncoder();
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == itemBack) {
        break;
      } else if (sel == itemEncInv) {
        applyEncoderInvertSetting(g_encoderInvertEnabled ? 0 : 1);
        saveEEPROM(g_storedVar);
        needRedraw = true;
      } else if (sel == itemExtPot) {
        showExtPotSettings();
        if (isEscapeToMainRequested()) break;
        resumeAfterChild();
        continue;
      } else if (sel == itemTrigger) {
        showTriggerSensorSettings();
        if (isEscapeToMainRequested()) break;
        resumeAfterChild();
        continue;
      } else if (sel == itemTest) {
        showSelfTest();
        if (isEscapeToMainRequested()) break;
        resumeAfterChild();
        continue;
      }
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
      char familyBuf[16];
      HAL_GetTriggerSensorFamilyLabel(familyBuf, sizeof(familyBuf));

      for (uint8_t i = 0; i < HARDWARE_ITEMS_COUNT; i++) {
        bool isSelected = (sel == i);
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)HARDWARE_MENU_LABELS[lang][i],
                       menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);

        const char* value = nullptr;
        if (i == itemEncInv) {
          value = getOnOffLabel(lang, g_encoderInvertEnabled ? 1 : 0);
        } else if (i == itemTrigger) {
          value = familyBuf;
        }

        if (value != nullptr && value[0] != '\0') {
          snprintf(msgStr, sizeof(msgStr), "%s", value);
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
