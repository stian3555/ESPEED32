#include "settings_display_submenus.h"
#include <Arduino.h>
#include "slot_ESC.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern char msgStr[50];
extern AiEsp32RotaryEncoder g_rotaryEncoder;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();

extern void showScreensaver();
extern void displayStatusLine();
extern void saveEEPROM(StoredVar_type toSave);

/**
 * Character-by-character text editor for screensaver lines.
 * Inspired by the rename car editor but for longer strings (up to 21 chars).
 * Encoder moves cursor in SELECT_OPTION mode, changes char value in SELECT_CHAR mode.
 * Encoder click = toggle mode / confirm when on OK.
 * Brake button = cancel (no save).
 */
static void editScreensaverText(char* text, const char* title) {
  const uint8_t TEXT_COLS = SCREENSAVER_TEXT_MAX - 1;  /* 21 visible positions */

  /* Working copy padded to full width with spaces */
  char tmpText[SCREENSAVER_TEXT_MAX];
  memset(tmpText, ' ', TEXT_COLS);
  tmpText[TEXT_COLS] = '\0';
  uint8_t origLen = strnlen(text, TEXT_COLS);
  memcpy(tmpText, text, origLen);

  uint8_t editMode = RENAME_CAR_SELECT_OPTION_MODE;  /* 0=move cursor, 1=edit char */
  uint8_t cursorPos = 0;

  obdFill(&g_obd, OBD_WHITE, 1);

  /* Static elements */
  uint8_t titleW = strnlen(title, 16) * 8;
  obdWriteString(&g_obd, 0, (OLED_WIDTH - titleW) / 2, 0, (char *)title, FONT_8x8, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, OLED_WIDTH - 24, 48, (char *)"OK", FONT_12x16, OBD_BLACK, 1);

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(0, TEXT_COLS, false);  /* 0..20 = chars, 21 = OK */
  resetUiEncoder(0);

  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  while (true) {
    /* Screensaver handling */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = readUiEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          ep != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        obdWriteString(&g_obd, 0, (OLED_WIDTH - titleW) / 2, 0, (char *)title, FONT_8x8, OBD_WHITE, 1);
        obdWriteString(&g_obd, 0, OLED_WIDTH - 24, 48, (char *)"OK", FONT_12x16, OBD_BLACK, 1);
      }
    }

    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        screensaverActive = true;
        screensaverEncoderPos = readUiEncoder();
        showScreensaver();
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
          obdWriteString(&g_obd, 0, (OLED_WIDTH - titleW) / 2, 0, (char *)title, FONT_8x8, OBD_WHITE, 1);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 24, 48, (char *)"OK", FONT_12x16, OBD_BLACK, 1);
        }
        delay(10);
        continue;
      }
    }

    /* Brake button = cancel */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
      obdFill(&g_obd, OBD_WHITE, 1);
      return;  /* Do NOT save */
    }

    /* Encoder changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (editMode == RENAME_CAR_SELECT_OPTION_MODE) {
        cursorPos = (uint8_t)readUiEncoder();
      } else {
        tmpText[cursorPos] = (char)readUiEncoder();
      }
    }

    /* Draw all 21 text characters - selected char inverted */
    for (uint8_t c = 0; c < TEXT_COLS; c++) {
      sprintf(msgStr, "%c", tmpText[c]);
      bool isCursor = (editMode == RENAME_CAR_SELECT_OPTION_MODE && cursorPos == c);
      obdWriteString(&g_obd, 0, c * 6, 16, msgStr, FONT_6x8, isCursor ? OBD_WHITE : OBD_BLACK, 1);
    }

    /* Draw arrows above/below cursor when editing a character */
    if (editMode == RENAME_CAR_SELECT_CHAR_MODE) {
      uint8_t cx = cursorPos * 6 + 2;
      for (uint8_t j = 0; j < 4; j++) {
        obdDrawLine(&g_obd, cx - j, 11 - j, cx + j, 11 - j, OBD_BLACK, 1);  /* up arrow */
        obdDrawLine(&g_obd, cx - j, 26 + j, cx + j, 26 + j, OBD_BLACK, 1);  /* down arrow */
      }
    } else {
      /* Clear arrow area */
      obdWriteString(&g_obd, 0, 0, 8,  (char *)"                     ", FONT_6x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 0, 24, (char *)"                     ", FONT_6x8, OBD_BLACK, 1);
    }

    /* OK button - highlighted when cursor is at TEXT_COLS position */
    obdWriteString(&g_obd, 0, OLED_WIDTH - 24, 48, (char *)"OK",
                   FONT_12x16, (cursorPos == TEXT_COLS) ? OBD_WHITE : OBD_BLACK, 1);

    /* Encoder button */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();

      if (cursorPos == TEXT_COLS) {
        /* OK selected - trim trailing spaces and save */
        int8_t last = (int8_t)TEXT_COLS - 1;
        while (last >= 0 && tmpText[last] == ' ') last--;
        tmpText[last + 1] = '\0';
        strncpy(text, tmpText, SCREENSAVER_TEXT_MAX);
        text[SCREENSAVER_TEXT_MAX - 1] = '\0';
        obdFill(&g_obd, OBD_WHITE, 1);
        return;
      }

      if (editMode == RENAME_CAR_SELECT_OPTION_MODE) {
        editMode = RENAME_CAR_SELECT_CHAR_MODE;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        setUiEncoderBoundaries(RENAME_CAR_MIN_ASCII, RENAME_CAR_MAX_ASCII, false);
        resetUiEncoder((uint8_t)tmpText[cursorPos]);
      } else {
        editMode = RENAME_CAR_SELECT_OPTION_MODE;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        setUiEncoderBoundaries(0, TEXT_COLS, false);
        resetUiEncoder(cursorPos);
      }
    }

    delay(10);
  }
}


/**
 * Screensaver settings submenu.
 * Shows LINE1, LINE2, TIME and BACK.
 * LINE1/LINE2 open the text editor. TIME edits the timeout inline.
 * Brake button or BACK = return to settings.
 */
void showScreensaverSettings() {
  const uint8_t SS_ITEMS = 5;
  uint8_t lang = g_storedVar.language;

  const char* ssNamesByLang[7][SS_ITEMS] = {
    {"NA", "LINJE1", "LINJE2", "TID", "TILBAKE"},
    {"NOW", "LINE1", "LINE2", "TIME", "BACK"},
    {"NOW", "LINE1", "LINE2", "TIME", "BACK"},
    {"NOW", "LINE1", "LINE2", "TIME", "BACK"},
    {"AHORA", "LINEA1", "LINEA2", "TIEMPO", "ATRAS"},
    {"JETZT", "ZEILE1", "ZEILE2", "ZEIT", "ZURUCK"},
    {"ORA", "RIGA1", "RIGA2", "TEMPO", "INDIETRO"}
  };
  const char* editorTitleL1ByLang[7] = {"Linje 1", "Line 1", "Line 1", "Line 1", "Linea 1", "Zeile 1", "Riga 1"};
  const char* editorTitleL2ByLang[7] = {"Linje 2", "Line 2", "Line 2", "Line 2", "Linea 2", "Zeile 2", "Riga 2"};
  const char* offLabelByLang[7] = {"AV", "OFF", "OFF", "OFF", "OFF", "AUS", "OFF"};

  const char** ssNames = ssNamesByLang[lang];
  const char* editorTitleL1 = editorTitleL1ByLang[lang];
  const char* editorTitleL2 = editorTitleL2ByLang[lang];

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(1, SS_ITEMS, false);
  resetUiEncoder(1);

  uint8_t sel = 1;
  uint8_t prevSel = 0xFF;
  bool inTimeEdit = false;
  uint16_t origTimeout = g_storedVar.screensaverTimeout;

  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  while (true) {
    /* Screensaver handling */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = readUiEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          ep != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;  /* Force redraw */
      }
    }
    if (consumeScreensaverWakeInput(wakeUp)) { continue; }

    if (!wakeUp && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = readUiEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel = 0xFF;  /* Force redraw after waking from power-save */
        }
        delay(10);
        continue;
      }
    }
    /* Also skip draw code if screensaver is active (e.g. activated via NOW button) */
    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      }
      delay(10);
      continue;
    }

    /* Brake button = cancel time edit / exit submenu */
    static bool brakePressedSS = false;
    static uint32_t lastBrakeSS = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakePressedSS && millis() - lastBrakeSS > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakePressedSS = true;
        lastBrakeSS = millis();
        lastInteraction = millis();
        if (inTimeEdit) {
          g_storedVar.screensaverTimeout = origTimeout;  /* Cancel - restore */
          inTimeEdit = false;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          setUiEncoderBoundaries(1, SS_ITEMS, false);
          resetUiEncoder(4);  /* Return to TIME item */
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel = 0xFF;
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;  /* Exit to settings menu */
        }
      }
    } else {
      brakePressedSS = false;
    }

    /* Encoder */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (inTimeEdit) {
        g_storedVar.screensaverTimeout = readUiEncoder();
        prevSel = 0xFF;  /* Redraw TIME row value */
      } else {
        sel = (uint8_t)readUiEncoder();
      }
    }

    /* Draw items if changed */
    if (sel != prevSel || inTimeEdit) {
      for (uint8_t idx = 0; idx < SS_ITEMS; idx++) {
        uint8_t yPx = (idx + 1) * HEIGHT8x8;  /* y=8,16,24,32,40 */
        bool isSelected = (sel == idx + 1);
        bool isEditingThis = (inTimeEdit && idx == 3);  /* TIME row */

        /* Item name */
        obdWriteString(&g_obd, 0, 0, yPx, (char *)ssNames[idx], FONT_8x8,
                       (isSelected || isEditingThis) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value on right side in FONT_6x8 */
        if (idx == 1) {
          /* LINE1: show first 10 chars, right-justified */
          snprintf(msgStr, 11, "%10s", g_storedVar.screensaverLine1);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, yPx, msgStr, FONT_6x8, OBD_BLACK, 1);
        } else if (idx == 2) {
          /* LINE2: show first 10 chars, right-justified */
          snprintf(msgStr, 11, "%10s", g_storedVar.screensaverLine2);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, yPx, msgStr, FONT_6x8, OBD_BLACK, 1);
        } else if (idx == 3) {
          /* TIME: show current timeout, right-justified (4 chars × 6px = 24px from right) */
          if (g_storedVar.screensaverTimeout == 0) {
            sprintf(msgStr, "%4s", offLabelByLang[lang]);
          } else {
            sprintf(msgStr, "%3ds", g_storedVar.screensaverTimeout);
          }
          obdWriteString(&g_obd, 0, OLED_WIDTH - 24, yPx, msgStr, FONT_6x8,
                         isEditingThis ? OBD_WHITE : OBD_BLACK, 1);
        }
        /* idx==0 (NOW) and idx==4 (BACK) have no value - name only */
      }
      prevSel = sel;
    }

    /* Encoder button */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (inTimeEdit) {
        /* Confirm timeout value */
        inTimeEdit = false;
        saveEEPROM(g_storedVar);
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        setUiEncoderBoundaries(1, SS_ITEMS, false);
        resetUiEncoder(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 2) {
        editScreensaverText(g_storedVar.screensaverLine1, editorTitleL1);
        saveEEPROM(g_storedVar);
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        setUiEncoderBoundaries(1, SS_ITEMS, false);
        resetUiEncoder(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 3) {
        editScreensaverText(g_storedVar.screensaverLine2, editorTitleL2);
        saveEEPROM(g_storedVar);
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        setUiEncoderBoundaries(1, SS_ITEMS, false);
        resetUiEncoder(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 4) {
        /* Enter TIME edit mode */
        inTimeEdit = true;
        origTimeout = g_storedVar.screensaverTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        setUiEncoderBoundaries(0, SCREENSAVER_TIMEOUT_MAX, false);
        resetUiEncoder(g_storedVar.screensaverTimeout);
      } else if (sel == 1) {
        /* NOW: activate screensaver immediately */
        screensaverActive = true;
        screensaverEncoderPos = readUiEncoder();
        showScreensaver();
        lastInteraction = millis();
      } else if (sel == 5) {
        break;  /* BACK */
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * Status line slot settings submenu.
 * Items: SLOT 1-4 (select content per fixed column) + BACK.
 * Live preview of the status bar is shown at y=56 while browsing and editing.
 * Follows the same ITEM_SELECTION / VALUE_SELECTION pattern as showSettingsMenu().
 */
void showStatusSettings() {
  /* ST_ITEMS: 4 slots + BACK */
  const uint8_t ST_ITEMS    = STATUS_SLOTS + 1;
  /* Slot content range: STATUS_BLANK..STATUS_CURRENT_MA */
  const uint8_t ST_SLOT_MAX = STATUS_CURRENT_MA;
  const uint8_t ST_LABEL_CHARS = 7;
  const uint8_t ST_LABEL_PIXELS = ST_LABEL_CHARS * 6;

  uint8_t lang = g_storedVar.language;

  /* Row labels */
  const char* rowNamesByLang[7][ST_ITEMS] = {
    {"FELT 1", "FELT 2", "FELT 3", "FELT 4", "TILBAKE"},
    {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4", "BACK"},
    {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4", "BACK"},
    {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4", "BACK"},
    {"CAMPO 1", "CAMPO 2", "CAMPO 3", "CAMPO 4", "ATRAS"},
    {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4", "ZURUCK"},
    {"SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4", "INDIETRO"}
  };
  const char** rowNames = rowNamesByLang[lang];

  /* Content type labels shown right-justified in menu. */
  const char* slotLabelsByLang[7][ST_SLOT_MAX + 1] = {
    {"---", "OUT%", "GASS", "BIL", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "THRO", "CAR", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "THRO", "CAR", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "THRO", "CAR", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "GAS", "AUTO", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "GAS", "AUTO", "CURR_A", "VOLT", "CURR_mA"},
    {"---", "OUT%", "GAS", "AUTO", "CURR_A", "VOLT", "CURR_mA"}
  };
  const char** slotLabels = slotLabelsByLang[lang];

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  setUiEncoderBoundaries(1, ST_ITEMS, false);
  resetUiEncoder(1);

  uint8_t sel          = 1;
  uint8_t prevSel      = 0xFF;
  MenuState_enum state = ITEM_SELECTION;
  uint16_t origValue   = 0;
  bool forceRedraw     = false;

  uint32_t lastInteraction   = millis();
  bool screensaverActive     = false;
  uint16_t screensaverEncPos = 0;

  while (true) {
    /* Screensaver handling */
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

    /* Brake button */
    static bool brakeInStatus = false;
    static uint32_t lastBrakeStatus = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeInStatus && millis() - lastBrakeStatus > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeInStatus = true;
        lastBrakeStatus = millis();
        if (state == VALUE_SELECTION) {
          /* Cancel: restore original value */
          g_storedVar.statusSlot[sel - 1] = origValue;
          state = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          setUiEncoderBoundaries(1, ST_ITEMS, false);
          resetUiEncoder(sel);
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel     = 0xFF;
          forceRedraw = true;
        } else {
          /* Wait for release so brake does not propagate to showSettingsMenu */
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeInStatus = false;
    }

    /* Encoder scroll */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      uint16_t ep = (uint16_t)readUiEncoder();
      if (state == ITEM_SELECTION) {
        sel = (uint8_t)ep;
      } else {
        g_storedVar.statusSlot[sel - 1] = ep;
        forceRedraw = true;
      }
    }

    /* Draw all items when selection or a slot value changes */
    if (sel != prevSel || forceRedraw) {
      forceRedraw = false;
      for (uint8_t idx = 0; idx < ST_ITEMS; idx++) {
        uint8_t yPx     = (idx + 1) * HEIGHT8x8;  /* y = 8, 16, 24, 32, 40 */
        bool isSelected = (sel == idx + 1);
        bool inEdit     = (state == VALUE_SELECTION && isSelected);

        /* Row label: inverted when selected in ITEM_SELECTION mode */
        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowNames[idx], FONT_8x8,
                       (isSelected && state == ITEM_SELECTION) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value label right-justified in a wider field for longer slot names. */
        if (idx < STATUS_SLOTS) {
          uint16_t v = g_storedVar.statusSlot[idx];
          const char* lbl = (v <= ST_SLOT_MAX) ? slotLabels[v] : "???";
          char vbuf[ST_LABEL_CHARS + 1];
          snprintf(vbuf, sizeof(vbuf), "%*s", ST_LABEL_CHARS, lbl);
          obdWriteString(&g_obd, 0, OLED_WIDTH - ST_LABEL_PIXELS, yPx, vbuf, FONT_6x8,
                         inEdit ? OBD_WHITE : OBD_BLACK, 1);
        }
      }

      /* Live preview at y=56: show how the status bar will actually look */
      displayStatusLine();

      prevSel = sel;
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (state == ITEM_SELECTION) {
        if (sel == ST_ITEMS) {
          /* BACK */
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
        /* Enter value selection for this slot */
        origValue = g_storedVar.statusSlot[sel - 1];
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        setUiEncoderBoundaries(0, ST_SLOT_MAX, false);
        resetUiEncoder(origValue);
        state       = VALUE_SELECTION;
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      } else {
        /* Confirm: save and return to item selection */
        saveEEPROM(g_storedVar);
        state = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        setUiEncoderBoundaries(1, ST_ITEMS, false);
        resetUiEncoder(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
}
