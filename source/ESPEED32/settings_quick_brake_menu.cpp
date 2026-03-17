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

enum ReleaseBrakeRowType : uint8_t {
  QB_ROW_MODE = 0,
  QB_ROW_ZONE = 1,
  QB_ROW_LEVEL = 2,
  QB_ROW_BACK = 3
};

static uint8_t getReleaseBrakeMenuItemCount(uint16_t mode) {
  return (mode == RELEASE_BRAKE_OFF) ? 2 : 4;
}

static ReleaseBrakeRowType getReleaseBrakeRowType(uint16_t mode, uint8_t visibleIndex) {
  if (visibleIndex <= 1) {
    return QB_ROW_MODE;
  }
  if (mode == RELEASE_BRAKE_OFF) {
    return QB_ROW_BACK;
  }
  if (visibleIndex == 2) {
    return QB_ROW_ZONE;
  }
  if (visibleIndex == 3) {
    return QB_ROW_LEVEL;
  }
  return QB_ROW_BACK;
}

static const char* getReleaseBrakeRowLabel(uint8_t lang, uint16_t mode, ReleaseBrakeRowType rowType) {
  static const char* MODE_LABELS[7]  = {"Modus", "Mode", "Mode", "Mode", "Modo", "Modus", "Modo"};
  static const char* ZONE_LABELS[7]  = {"Sone", "Zone", "Zone", "Zone", "Zona", "Zone", "Zona"};
  static const char* QUICK_LABELS[7] = {"Brems", "Brake", "Brake", "Brake", "Freno", "Bremse", "Freno"};
  static const char* DRAG_LABELS[7]  = {"Drag", "Drag", "Drag", "Drag", "Drag", "Drag", "Drag"};
  static const char* BACK_LABELS[7]  = {"Tilbake", "Back", "Back", "Back", "Atras", "Zuruck", "Indietro"};

  switch (rowType) {
    case QB_ROW_MODE:
      return MODE_LABELS[lang];
    case QB_ROW_ZONE:
      return ZONE_LABELS[lang];
    case QB_ROW_LEVEL:
      return (mode == RELEASE_BRAKE_DRAG) ? DRAG_LABELS[lang] : QUICK_LABELS[lang];
    case QB_ROW_BACK:
    default:
      return BACK_LABELS[lang];
  }
}

static const char* getReleaseBrakeMenuModeValue(uint8_t lang, uint16_t mode) {
  static const char* MODE_VALUES[7][3] = {
    {"AV", "QUICK", "DRAG"},
    {"OFF", "QUICK", "DRAG"},
    {"OFF", "QUICK", "DRAG"},
    {"OFF", "QUICK", "DRAG"},
    {"OFF", "QUICK", "DRAG"},
    {"AUS", "QUICK", "DRAG"},
    {"OFF", "QUICK", "DRAG"}
  };

  if (mode > RELEASE_BRAKE_DRAG) {
    mode = RELEASE_BRAKE_OFF;
  }
  return MODE_VALUES[lang][mode];
}

/**
 * Release Brake submenu.
 * Opened by clicking the release-brake item in the main menu.
 * Items: MODE (OFF/QUICK/DRAG), optional ZONE + BRAKE/DRAG level, BACK.
 * QUICK cuts output to brake inside the zone. DRAG keeps output active and
 * blends in drag only while the trigger is moving toward release.
 */
void showQuickBrakeMenu() {
  uint8_t lang = g_storedVar.language;

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(1, getReleaseBrakeMenuItemCount(g_storedVar.carParam[g_carSel].quickBrakeEnabled), false);
  resetUiEncoder(1);

  uint8_t sel          = 1;
  uint8_t prevSel      = 0xFF;
  MenuState_enum state = ITEM_SELECTION;
  uint16_t origValue   = 0;
  ReleaseBrakeRowType editRowType = QB_ROW_MODE;
  bool forceRedraw     = true;

  uint32_t lastInteraction   = millis();
  bool screensaverActive     = false;
  uint16_t screensaverEncPos = 0;

  static bool brakeInQB = false;
  static uint32_t lastBrakeQB = 0;

  auto currentMode = [&]() -> uint16_t {
    return g_storedVar.carParam[g_carSel].quickBrakeEnabled;
  };

  auto refreshSelectionBounds = [&]() {
    uint8_t visibleItems = getReleaseBrakeMenuItemCount(currentMode());
    if (sel > visibleItems) {
      sel = visibleItems;
    }
    setUiEncoderBoundaries(1, visibleItems, false);
    resetUiEncoder(sel);
  };

  while (true) {
    /* Screensaver */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = readUiEncoder();
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
          screensaverEncPos = readUiEncoder();
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
          if (editRowType == QB_ROW_MODE) g_storedVar.carParam[g_carSel].quickBrakeEnabled = origValue;
          else if (editRowType == QB_ROW_ZONE) g_storedVar.carParam[g_carSel].quickBrakeThreshold = origValue;
          else if (editRowType == QB_ROW_LEVEL) g_storedVar.carParam[g_carSel].quickBrakeStrength = origValue;
          state = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          refreshSelectionBounds();
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
      uint16_t ep = (uint16_t)readUiEncoder();
      if (state == ITEM_SELECTION) {
        sel = (uint8_t)ep;
        forceRedraw = true;
      } else {
        /* Live update while editing */
        if (editRowType == QB_ROW_MODE) g_storedVar.carParam[g_carSel].quickBrakeEnabled = ep;
        else if (editRowType == QB_ROW_ZONE) g_storedVar.carParam[g_carSel].quickBrakeThreshold = ep;
        else if (editRowType == QB_ROW_LEVEL) g_storedVar.carParam[g_carSel].quickBrakeStrength = ep;
        forceRedraw = true;
      }
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (state == ITEM_SELECTION) {
        ReleaseBrakeRowType rowType = getReleaseBrakeRowType(currentMode(), sel);
        if (rowType == QB_ROW_BACK) {
          /* BACK */
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
        /* Enter value edit */
        uint16_t minV = 0, maxV = 1, curV = 0;
        if (rowType == QB_ROW_MODE) { curV = g_storedVar.carParam[g_carSel].quickBrakeEnabled; minV = RELEASE_BRAKE_OFF; maxV = RELEASE_BRAKE_DRAG; }
        else if (rowType == QB_ROW_ZONE) { curV = g_storedVar.carParam[g_carSel].quickBrakeThreshold; minV = 0; maxV = QUICK_BRAKE_THRESHOLD_MAX; }
        else if (rowType == QB_ROW_LEVEL) { curV = g_storedVar.carParam[g_carSel].quickBrakeStrength; minV = 0; maxV = QUICK_BRAKE_STRENGTH_MAX; }
        origValue = curV;
        editRowType = rowType;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        setUiEncoderBoundaries(minV, maxV, false);
        resetUiEncoder(curV);
        state       = VALUE_SELECTION;
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      } else {
        /* Confirm value */
        saveEEPROM(g_storedVar);
        state = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        refreshSelectionBounds();
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }

    /* Draw */
    if (forceRedraw || sel != prevSel) {
      forceRedraw = false;
      prevSel = sel;
      uint16_t mode = currentMode();
      uint8_t visibleItems = getReleaseBrakeMenuItemCount(mode);
      obdFill(&g_obd, OBD_WHITE, 1);

      for (uint8_t idx = 0; idx < visibleItems; idx++) {
        uint8_t yPx     = idx * HEIGHT8x8;
        bool isSelected = (sel == idx + 1);
        bool inEdit     = (state == VALUE_SELECTION && isSelected);
        ReleaseBrakeRowType rowType = getReleaseBrakeRowType(mode, idx + 1);
        const char* rowName = getReleaseBrakeRowLabel(lang, mode, rowType);

        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowName, FONT_8x8,
                       (isSelected && state == ITEM_SELECTION) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value right-justified */
        char vbuf[12];
        if (rowType == QB_ROW_MODE) {
          snprintf(vbuf, sizeof(vbuf), "%s", getReleaseBrakeMenuModeValue(lang, mode));
        } else if (rowType == QB_ROW_ZONE) {
          snprintf(vbuf, sizeof(vbuf), "%3d%%", g_storedVar.carParam[g_carSel].quickBrakeThreshold);
        } else if (rowType == QB_ROW_LEVEL) {
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
