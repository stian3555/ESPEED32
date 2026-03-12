#include "menu_car.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_strings.h"
#include "ui_text_access.h"
#include "settings_reset_menu.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern uint16_t g_carSel;
extern Menu_type g_carMenu;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern char msgStr[50];
extern bool g_isEditingCarSelection;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern bool checkRaceModeEscape();
extern void requestEscapeToMain();
extern bool isEscapeToMainRequested();
extern uint8_t getMainMenuSelector();
extern uint8_t getMainMenuItemsCount();

extern void showScreensaver();
extern void initMenuItems();
extern void saveEEPROM(StoredVar_type toSave);

void showCarSelection() {
  /* "Frame" indicates which items are currently displayed.
     It consist of a lower and upper bound: only the items within this boundaries are displayed.
     The difference between upper and lower bound is fixed to be g_carMenu.lines
     It's important that the encoder boundaries matches the menu items (e.g., 10 items (cars), encoder boundaries must be [0,9]) */
  static uint16_t frameUpper = 1;
  static uint16_t frameLower = g_carMenu.lines;

  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Save original value for cancel */
  uint16_t originalCarNumber = g_storedVar.selectedCarNumber;

  /* Set encoder to car selection parameter */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, CAR_MAX_COUNT - 1, false);
  g_rotaryEncoder.reset(g_storedVar.selectedCarNumber);

  /* Screensaver support */
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  /* Exit car selection when encoder is clicked */
  while (true)
  {
    /* Calculate throttle percentage for screensaver wake-up */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Check for any wake-up input first (encoder, button, throttle) */
    bool wakeUpTriggered = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Check for screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 && millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);  /* Small delay to prevent watchdog timeout */
        continue;  /* Don't draw menu while screensaver is active */
      }
    }

    /* Check for brake button press - cancel and exit */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
      g_storedVar.selectedCarNumber = originalCarNumber;  /* Restore original */
      obdFill(&g_obd, OBD_WHITE, 1);
      return;
    }

    /* Get encoder value if changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      g_storedVar.selectedCarNumber = g_rotaryEncoder.readEncoder();
    }

    /* If encoder move out of frame, adjust frame */
    if (g_storedVar.selectedCarNumber > frameLower)
    {
      frameLower = g_storedVar.selectedCarNumber;
      frameUpper = frameLower - g_carMenu.lines + 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }
    else if (g_storedVar.selectedCarNumber < frameUpper)
    {
      frameUpper = g_storedVar.selectedCarNumber;
      frameLower = frameUpper + g_carMenu.lines - 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }

    /* Print car menu */
    for (uint8_t i = 0; i < g_carMenu.lines; i++)
    {
      /* Print the item (car) name */
      obdWriteString(&g_obd, 0, 0, i * HEIGHT12x16, g_carMenu.item[frameUpper + i].name, FONT_12x16, (g_storedVar.selectedCarNumber - frameUpper == i) ? OBD_WHITE : OBD_BLACK, 1);
      if (g_carMenu.item[frameUpper + i].value != ITEM_NO_VALUE)
      {
        /* value is a generic pointer to void, so first cast to uint16_t pointer, then take the pointed value */
        sprintf(msgStr, "%2d", *(uint16_t *)(g_carMenu.item[frameUpper + i].value));
        /* Print the item value (car number) */
        obdWriteString(&g_obd, 0, OLED_WIDTH - 24, i * HEIGHT12x16, msgStr, FONT_12x16, OBD_BLACK, 1);
      }
    }

    /* Print car selection label on the bottom of the screen */
    const char* selLabel = STR_SELECT_CAR[g_storedVar.language];
    int selLabelWidth = strlen(selLabel) * 6;
    obdWriteString(&g_obd, 0, (OLED_WIDTH - selLabelWidth) / 2, OLED_HEIGHT - HEIGHT8x8, (char *)selLabel, FONT_6x8, OBD_WHITE, 1);

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); return; }
  }

  /* Reset encoder and clear is done in caller routine */
  return;
}


/**
 * Show the Copy Car Settings screen. Allows copying all parameters from one car to another.
 */
void showCopyCarSettings() {
  static uint16_t frameUpper = 1;
  static uint16_t frameLower = g_carMenu.lines;
  uint16_t sourceCar = 0;
  uint16_t destCar = 0;

  /* ========== SELECT SOURCE CAR (FROM) ========== */
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Screensaver support */
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  /* Set encoder to car selection parameter */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, CAR_MAX_COUNT - 1, false);
  g_rotaryEncoder.reset(g_storedVar.selectedCarNumber);
  sourceCar = g_storedVar.selectedCarNumber;

  /* Select source car */
  while (true)
  {
    /* Calculate throttle percentage for screensaver wake-up */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Check for any wake-up input first (encoder, button, throttle) */
    bool wakeUpTriggered = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Check for screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 && millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);  /* Small delay to prevent watchdog timeout */
        continue;  /* Don't draw menu while screensaver is active */
      }
    }

    /* Check for brake button press - cancel and exit */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
      obdFill(&g_obd, OBD_WHITE, 1);
      return;
    }

    /* Get encoder value if changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      sourceCar = g_rotaryEncoder.readEncoder();
    }

    /* If encoder move out of frame, adjust frame */
    if (sourceCar > frameLower)
    {
      frameLower = sourceCar;
      frameUpper = frameLower - g_carMenu.lines + 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }
    else if (sourceCar < frameUpper)
    {
      frameUpper = sourceCar;
      frameLower = frameUpper + g_carMenu.lines - 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }

    /* Print car menu */
    for (uint8_t i = 0; i < g_carMenu.lines; i++)
    {
      /* Print the item (car) name */
      obdWriteString(&g_obd, 0, 0, i * HEIGHT12x16, g_carMenu.item[frameUpper + i].name, FONT_12x16, (sourceCar - frameUpper == i) ? OBD_WHITE : OBD_BLACK, 1);
      if (g_carMenu.item[frameUpper + i].value != ITEM_NO_VALUE)
      {
        /* Print the item value (car number) */
        sprintf(msgStr, "%2d", *(uint16_t *)(g_carMenu.item[frameUpper + i].value));
        obdWriteString(&g_obd, 0, OLED_WIDTH - 24, i * HEIGHT12x16, msgStr, FONT_12x16, OBD_BLACK, 1);
      }
    }

    /* Print "COPY FROM:" on the bottom of the screen */
    const char* fromLabel = STR_COPY_FROM[g_storedVar.language];
    int fromLabelWidth = strlen(fromLabel) * 6;
    obdWriteString(&g_obd, 0, (OLED_WIDTH - fromLabelWidth) / 2, OLED_HEIGHT - HEIGHT8x8, (char *)fromLabel, FONT_6x8, OBD_WHITE, 1);

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); return; }
  }

  /* Small delay to prevent double-click */
  delay(200);
  if (isEscapeToMainRequested()) return;

  /* ========== SELECT DESTINATION CAR (TO) ========== */
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Reset screensaver tracking */
  lastInteraction = millis();
  screensaverActive = false;

  /* Reset encoder for destination car selection (CAR_MAX_COUNT = ALL option) */
  g_rotaryEncoder.setBoundaries(0, CAR_MAX_COUNT, false);  /* 0-19 = cars, 20 = ALL */
  g_rotaryEncoder.reset(g_storedVar.selectedCarNumber);
  destCar = g_storedVar.selectedCarNumber;

  /* Reset frame if needed */
  frameUpper = 1;
  frameLower = g_carMenu.lines;

  uint8_t visibleLines = g_carMenu.lines;
  const uint16_t ALL_CARS_OPTION = CAR_MAX_COUNT;  /* Index for "ALL" option */

  /* Select destination car */
  while (true)
  {
    /* Calculate throttle percentage for screensaver wake-up */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Check for any wake-up input first (encoder, button, throttle) */
    bool wakeUpTriggered = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Check for screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 && millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);  /* Small delay to prevent watchdog timeout */
        continue;  /* Don't draw menu while screensaver is active */
      }
    }

    /* Check for brake button press - cancel and exit */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
      obdFill(&g_obd, OBD_WHITE, 1);
      return;
    }

    /* Get encoder value if changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      destCar = g_rotaryEncoder.readEncoder();
    }

    /* If encoder move out of frame, adjust frame */
    if (destCar > frameLower)
    {
      frameLower = destCar;
      frameUpper = frameLower - visibleLines + 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }
    else if (destCar < frameUpper)
    {
      frameUpper = destCar;
      frameLower = frameUpper + visibleLines - 1;
      obdFill(&g_obd, OBD_WHITE, 1);
    }

    /* Print car menu */
    for (uint8_t i = 0; i < visibleLines; i++)
    {
      uint16_t itemIndex = frameUpper + i;
      bool isSelected = (destCar == itemIndex);

      if (itemIndex == ALL_CARS_OPTION) {
        /* Print "ALL" option */
        obdWriteString(&g_obd, 0, 0, i * HEIGHT12x16, (char *)STR_ALL[g_storedVar.language], FONT_12x16, isSelected ? OBD_WHITE : OBD_BLACK, 1);
      } else if (itemIndex < CAR_MAX_COUNT) {
        /* Print the item (car) name */
        obdWriteString(&g_obd, 0, 0, i * HEIGHT12x16, g_carMenu.item[itemIndex].name, FONT_12x16, isSelected ? OBD_WHITE : OBD_BLACK, 1);
        if (g_carMenu.item[itemIndex].value != ITEM_NO_VALUE)
        {
          /* Print the item value (car number) */
          sprintf(msgStr, "%2d", *(uint16_t *)(g_carMenu.item[itemIndex].value));
          obdWriteString(&g_obd, 0, OLED_WIDTH - 24, i * HEIGHT12x16, msgStr, FONT_12x16, OBD_BLACK, 1);
        }
      }
    }

    /* Print "COPY TO:" on the bottom of the screen */
    const char* toLabel = STR_COPY_TO[g_storedVar.language];
    int toLabelWidth = strlen(toLabel) * 6;
    obdWriteString(&g_obd, 0, (OLED_WIDTH - toLabelWidth) / 2, OLED_HEIGHT - HEIGHT8x8, (char *)toLabel, FONT_6x8, OBD_WHITE, 1);

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); return; }
  }

  /* Copy car parameters */
  if (destCar == ALL_CARS_OPTION) {
    /* Copy to ALL cars (except source car) */
    for (uint16_t i = 0; i < CAR_MAX_COUNT; i++) {
      if (i != sourceCar) {
        g_storedVar.carParam[i].minSpeed = g_storedVar.carParam[sourceCar].minSpeed;
        g_storedVar.carParam[i].brake = g_storedVar.carParam[sourceCar].brake;
        g_storedVar.carParam[i].maxSpeed = g_storedVar.carParam[sourceCar].maxSpeed;
        g_storedVar.carParam[i].throttleCurveVertex.inputThrottle = g_storedVar.carParam[sourceCar].throttleCurveVertex.inputThrottle;
        g_storedVar.carParam[i].throttleCurveVertex.curveSpeedDiff = g_storedVar.carParam[sourceCar].throttleCurveVertex.curveSpeedDiff;
        g_storedVar.carParam[i].antiSpin = g_storedVar.carParam[sourceCar].antiSpin;
        g_storedVar.carParam[i].freqPWM = g_storedVar.carParam[sourceCar].freqPWM;
        g_storedVar.carParam[i].brakeButtonReduction = g_storedVar.carParam[sourceCar].brakeButtonReduction;
        g_storedVar.carParam[i].quickBrakeEnabled = g_storedVar.carParam[sourceCar].quickBrakeEnabled;
        g_storedVar.carParam[i].quickBrakeThreshold = g_storedVar.carParam[sourceCar].quickBrakeThreshold;
        g_storedVar.carParam[i].quickBrakeStrength = g_storedVar.carParam[sourceCar].quickBrakeStrength;
      }
    }
    /* Show confirmation message */
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 0, 24, (char *)STR_COPIED_ALL[g_storedVar.language], FONT_12x16, OBD_BLACK, 1);
    delay(1000);
  } else if (sourceCar != destCar) {
    /* Copy to single car */
    g_storedVar.carParam[destCar].minSpeed = g_storedVar.carParam[sourceCar].minSpeed;
    g_storedVar.carParam[destCar].brake = g_storedVar.carParam[sourceCar].brake;
    g_storedVar.carParam[destCar].maxSpeed = g_storedVar.carParam[sourceCar].maxSpeed;
    g_storedVar.carParam[destCar].throttleCurveVertex.inputThrottle = g_storedVar.carParam[sourceCar].throttleCurveVertex.inputThrottle;
    g_storedVar.carParam[destCar].throttleCurveVertex.curveSpeedDiff = g_storedVar.carParam[sourceCar].throttleCurveVertex.curveSpeedDiff;
    g_storedVar.carParam[destCar].antiSpin = g_storedVar.carParam[sourceCar].antiSpin;
    g_storedVar.carParam[destCar].freqPWM = g_storedVar.carParam[sourceCar].freqPWM;
    g_storedVar.carParam[destCar].brakeButtonReduction = g_storedVar.carParam[sourceCar].brakeButtonReduction;
    g_storedVar.carParam[destCar].quickBrakeEnabled = g_storedVar.carParam[sourceCar].quickBrakeEnabled;
    g_storedVar.carParam[destCar].quickBrakeThreshold = g_storedVar.carParam[sourceCar].quickBrakeThreshold;
    g_storedVar.carParam[destCar].quickBrakeStrength = g_storedVar.carParam[sourceCar].quickBrakeStrength;

    /* Show confirmation message */
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 34, 24, (char *)STR_COPIED[g_storedVar.language], FONT_12x16, OBD_BLACK, 1);
    delay(1000);
  }

  /* Reset frame for next time */
  frameUpper = 1;
  frameLower = g_carMenu.lines;

  return;
}


/**
 * Show the options to Select or Rename the Car, called upon selecting the CAR item in the main menu
 */
void showSelectRenameCar() {

  /* Trigger reading stops, so stop the motor */
  /* Set trigRaw to max throttle if throttle is reversed, set to min throttle otherwise */
  /* g_escVar.trigger_raw = THROTTLE_REV ? g_storedVar.maxTrigger_raw : g_storedVar.minTrigger_raw; */ /*TODO: decide what to do now that trigger reading is done on task 2 */

  uint16_t selectedOption = 0;
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Set encoder to selection parameter */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, 5, false); /* Boundaries are [0, 5] because there are six options */
  g_rotaryEncoder.reset(selectedOption);

  /* Setup scrolling frame - number of visible items depends on font size */
  const uint8_t totalOptions = 6;
  uint8_t visibleLines = getMenuLines();
  uint8_t frameUpper = 0;
  uint8_t frameLower = visibleLines - 1;

  /* Track editing state for RACESWP option */
  bool isEditingRaceswp = false;
  uint16_t tempRaceswpValue = g_storedVar.gridCarSelectEnabled;

  /* Screensaver support */
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  /* Exit car selection when encoder is clicked */
  while (true)
  {
    /* Calculate throttle percentage for screensaver wake-up */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Check for any wake-up input first (encoder, button, throttle) */
    bool wakeUpTriggered = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Check for screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 && millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);  /* Small delay to prevent watchdog timeout */
        continue;  /* Don't draw menu while screensaver is active */
      }
    }

    /* Check for brake button press - acts as "back" in CAR menu */
    static bool brakeBtnInCarMenu = false;
    static uint32_t lastBrakeBtnCarMenuTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInCarMenu && millis() - lastBrakeBtnCarMenuTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInCarMenu = true;
        lastBrakeBtnCarMenuTime = millis();
        lastInteraction = millis();

        if (isEditingRaceswp) {
          /* Cancel RACESWP editing */
          isEditingRaceswp = false;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(0, 5, false);
          g_rotaryEncoder.reset(selectedOption);
        } else {
          /* Exit CAR menu completely */
          goto exitCarMenu;
        }
      }
    } else {
      brakeBtnInCarMenu = false;
    }

    /* Get encoder value if changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (!isEditingRaceswp) {
        selectedOption = g_rotaryEncoder.readEncoder();
      } else {
        /* When editing RACESWP, encoder changes the value */
        tempRaceswpValue = g_rotaryEncoder.readEncoder();
      }
    }

    /* Adjust frame if selection moves outside visible area (only when not editing) */
    if (!isEditingRaceswp) {
      if (selectedOption > frameLower)
      {
        frameLower = selectedOption;
        frameUpper = frameLower - visibleLines + 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      }
      else if (selectedOption < frameUpper)
      {
        frameUpper = selectedOption;
        frameLower = frameUpper + visibleLines - 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Determine font size and dimensions based on setting */
    uint8_t menuFont = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? FONT_8x8 : FONT_12x16;
    uint8_t charWidth = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? WIDTH8x8 : WIDTH12x16;
    uint8_t lineHeight = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? 8 : HEIGHT12x16;

    /* Display only the visible items within the frame */
    for (uint8_t i = 0; i < visibleLines; i++)
    {
      uint8_t optionIndex = frameUpper + i;
      if (optionIndex >= totalOptions) break;

      uint8_t yPos = i * lineHeight;
      bool isSelected = (optionIndex == selectedOption);

      uint16_t lang = g_storedVar.language;

      switch (optionIndex)
      {
        case CAR_OPTION_SELECT:
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 0), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          break;

        case CAR_OPTION_RENAME:
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 1), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          break;

        case CAR_OPTION_GRID_SEL:
        {
          /* Grid select option - show label and right-aligned ON/OFF value */
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 2), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          /* Show temp value when editing, stored value otherwise */
          uint16_t displayValue = isEditingRaceswp ? tempRaceswpValue : g_storedVar.gridCarSelectEnabled;
          sprintf(msgStr, "%3s", getOnOffLabel(lang, displayValue ? 1 : 0));
          /* Value is white when editing, follows selection otherwise */
          uint8_t valueColor = isEditingRaceswp ? OBD_WHITE : (isSelected ? OBD_WHITE : OBD_BLACK);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 3 * charWidth, yPos, msgStr, menuFont, valueColor, 1);
          break;
        }

        case CAR_OPTION_COPY:
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 3), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          break;

        case CAR_OPTION_RESET:
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 4), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          break;

        case CAR_OPTION_BACK:
        {
          /* Display BACK option using helper for text case support */
          obdWriteString(&g_obd, 0, 0, yPos, (char *)getBackLabel(lang), menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
          break;
        }
      }
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); goto exitCarMenu; }
  }

  /* Handle encoder click based on current state */
  if (selectedOption == CAR_OPTION_GRID_SEL && !isEditingRaceswp) {
    /* First click on RACESWP: enter edit mode */
    isEditingRaceswp = true;
    g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
    g_rotaryEncoder.setBoundaries(0, 1, false);  /* 0=OFF, 1=ON */
    g_rotaryEncoder.reset(tempRaceswpValue);
    /* Wait for second click to confirm */
    while (!g_rotaryEncoder.isEncoderButtonClicked()) {
      /* Check for brake button to cancel editing */
      if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
        /* Cancel editing - restore encoder for option selection */
        isEditingRaceswp = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, 5, false);
        g_rotaryEncoder.reset(selectedOption);
        obdFill(&g_obd, OBD_WHITE, 1);
        break;
      }

      tempRaceswpValue = g_rotaryEncoder.encoderChanged() ? g_rotaryEncoder.readEncoder() : tempRaceswpValue;

      /* Redraw the RACESWP option with updated value */
      uint8_t optionScreenPos = CAR_OPTION_GRID_SEL - frameUpper;
      if (optionScreenPos < visibleLines) {
        /* Determine font size and dimensions based on setting */
        uint8_t menuFont = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? FONT_8x8 : FONT_12x16;
        uint8_t charWidth = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? WIDTH8x8 : WIDTH12x16;
        uint8_t lineHeight = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? 8 : HEIGHT12x16;
        uint8_t yPos = optionScreenPos * lineHeight;
        uint16_t lang = g_storedVar.language;

        obdWriteString(&g_obd, 0, 0, yPos, (char *)getCarMenuOption(lang, 2), menuFont, OBD_WHITE, 1);
        sprintf(msgStr, "%3s", getOnOffLabel(lang, tempRaceswpValue ? 1 : 0));
        obdWriteString(&g_obd, 0, OLED_WIDTH - 3 * charWidth, yPos, msgStr, menuFont, OBD_WHITE, 1);
      }
    }
    /* Save the confirmed value (only if we didn't cancel) */
    if (isEditingRaceswp) {
      g_storedVar.gridCarSelectEnabled = tempRaceswpValue;
      selectedOption = CAR_OPTION_GRID_SEL;  /* Mark as handled */
    }
  }

  /* If RENAME option was selected, go to renameCar routine */
  if (selectedOption == CAR_OPTION_RENAME)
  {
    showRenameCar();
    if (!isEscapeToMainRequested()) saveEEPROM(g_storedVar);
  }
  /* If SELECT option was selected, go to showCarSelection routine */
  else if (selectedOption == CAR_OPTION_SELECT)
  {
    showCarSelection();
    if (!isEscapeToMainRequested()) saveEEPROM(g_storedVar);
  }
  /* If RACESWP option was selected, value was already saved in the edit loop above */
  else if (selectedOption == CAR_OPTION_GRID_SEL)
  {
    saveEEPROM(g_storedVar);
  }
  /* If COPY option was selected, go to showCopyCarSettings routine */
  else if (selectedOption == CAR_OPTION_COPY)
  {
    showCopyCarSettings();
    if (!isEscapeToMainRequested()) saveEEPROM(g_storedVar);
  }
  /* If RESET option was selected, reset all car parameters with double confirmation */
  else if (selectedOption == CAR_OPTION_RESET)
  {
    uint16_t lang = g_storedVar.language;
    const char* carLabel = (lang == LANG_NOR) ? "ALLE BILER" : "ALL CARS";
    bool confirmed = showResetConfirmDialog(carLabel);
    if (confirmed) {
      resetAllCarsToFactoryDefaults();
      saveEEPROM(g_storedVar);
      initMenuItems();
      obdFill(&g_obd, OBD_WHITE, 1);
      const char* doneText = (lang == LANG_NOR) ? "NULLSTILT!" : "RESET DONE!";
      int tw = strlen(doneText) * WIDTH12x16;
      obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 24, (char *)doneText, FONT_12x16, OBD_BLACK, 1);
      delay(1500);
      obdFill(&g_obd, OBD_WHITE, 1);
    }
  }
  /* If BACK option was selected, simply exit without changes */
  /* Note: No action needed for BACK, we just fall through to the reset code below */

exitCarMenu:
  /* Reset encoder */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, getMainMenuItemsCount(), false);
  g_rotaryEncoder.reset(getMainMenuSelector());
  g_escVar.encoderPos = getMainMenuSelector();
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  return;
}


/**
 * Show the Rename car screen. Called by selecting RENAME option on the showSelectRenameCar screen.
 */
void showRenameCar() {

  uint16_t selectedChar = RENAME_CAR_MIN_ASCII; /* the selected char: starts from 33 (ASCII for !) to 122 (ASCII for z)*/
  uint16_t selectedOption = 0;                  /* The selected option, could be one of the char of the name (0 : CAR_NAME_MAX_SIZE - 2) or the confirm option (CAR_NAME_MAX_SIZE - 1)*/
                                                /* Remember that CAR_NAME_MAX_SIZE includes the last terminator char */
  char tmpName[CAR_NAME_MAX_SIZE];  /* Array of chars containing the temporary name */
  uint16_t mode = RENAME_CAR_SELECT_OPTION_MODE;  /* Initial mode. There are two mode:
                                                     - RENAME_CAR_SELECT_OPTION_MODE, when scrolling the encoder changes the selectedOption (pick which char to change, or the OK option)
                                                     - RENAME_CAR_SELECT_CHAR_MODE, when scrolling the encoder changes the selectedChar, that is the value of the selectedOption */
  sprintf(tmpName, "%s", g_storedVar.carParam[g_storedVar.selectedCarNumber].carName); /* Store the current carName in the temporary name */

  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Set encoder to selection parameter */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, CAR_NAME_MAX_SIZE - 1, false);
  g_rotaryEncoder.reset(selectedOption);

  /* Print "-RENAME THE CAR-"  and "-CLICK OK TO CONFIRM" */
  obdWriteString(&g_obd, 0, 16, 0, (char *)STR_RENAME_CAR[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, 1, OLED_HEIGHT - HEIGHT8x8, (char *)STR_CONFIRM[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);

  /* Draw the right arrow */
  for (uint8_t j = 0; j < 8; j++)
  {
    obdDrawLine(&g_obd, 80 + j, 16 + j, 80 + j, 30 - j, OBD_BLACK, 1);
  }
  obdDrawLine(&g_obd, 72, 22, 80, 22, OBD_BLACK, 1);
  obdDrawLine(&g_obd, 72, 23, 80, 23, OBD_BLACK, 1);
  obdDrawLine(&g_obd, 72, 24, 80, 24, OBD_BLACK, 1);

  /* Screensaver support */
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  /* Exit car renaming when encoder is clicked AND CONFIRM is selected */
  while (1)
  {
    /* Calculate throttle percentage for screensaver wake-up */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Check for any wake-up input first (encoder, button, throttle) */
    bool wakeUpTriggered = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        /* Redraw static elements */
        obdWriteString(&g_obd, 0, 16, 0, (char *)STR_RENAME_CAR[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);
        obdWriteString(&g_obd, 0, 1, OLED_HEIGHT - HEIGHT8x8, (char *)STR_CONFIRM[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);
        for (uint8_t j = 0; j < 8; j++) {
          obdDrawLine(&g_obd, 80 + j, 16 + j, 80 + j, 30 - j, OBD_BLACK, 1);
        }
        obdDrawLine(&g_obd, 72, 22, 80, 22, OBD_BLACK, 1);
        obdDrawLine(&g_obd, 72, 23, 80, 23, OBD_BLACK, 1);
        obdDrawLine(&g_obd, 72, 24, 80, 24, OBD_BLACK, 1);
      }
    }

    /* Check for screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 && millis() - lastInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!screensaverActive) {
          screensaverActive = true;
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);  /* Small delay to prevent watchdog timeout */
        continue;  /* Don't draw menu while screensaver is active */
      }
    }

    /* Check for brake button press - cancel and exit */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
      /* Don't save changes - just exit */
      obdFill(&g_obd, OBD_WHITE, 1);
      return;
    }

    /* Get encoder value if changed */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      /* Change selectedOption if in RENAME_CAR_SELECT_OPTION_MODE */
      if (mode == RENAME_CAR_SELECT_OPTION_MODE) {
        selectedOption = g_rotaryEncoder.readEncoder();
      }
      /* Change selectedChar if in RENAME_CAR_SELECT_CHAR_MODE */
      if (mode == RENAME_CAR_SELECT_CHAR_MODE) {
        selectedChar = g_rotaryEncoder.readEncoder();
        tmpName[selectedOption] = (char)selectedChar; /* Change the value of the selected char in the temp name */

        /* Draw the upward and downward arrows on the selected char to indicate that it can be changed */
        for (uint8_t j = 0; j < 6; j++) {
          obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 14 - j, 11 - j + (selectedOption * 12), 14 - j, OBD_BLACK, 1);
          obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 33 + j, 11 - j + (selectedOption * 12), 33 + j, OBD_BLACK, 1);
        }
      }
    }

    /* Print each Char of the name, only the selected one is highlighted (WHITE color) */
    for (uint8_t i = 0; i < CAR_NAME_MAX_SIZE - 1; i++) 
    {
      sprintf(msgStr, "%c", tmpName[i]);
      obdWriteString(&g_obd, 0, 0 + (i * 12), 22, msgStr, FONT_12x16, (selectedOption == i) ? OBD_WHITE : OBD_BLACK, 1);
    }

    /* Print the confirm button */
    obdWriteString(&g_obd, 0, OLED_WIDTH - 24, 22, (char *)"OK", FONT_12x16, (selectedOption == CAR_NAME_MAX_SIZE - 1) ? OBD_WHITE : OBD_BLACK, 1);

    /* If encoder button is clicked */
    if (g_rotaryEncoder.isEncoderButtonClicked())
    {
      lastInteraction = millis();
      if (screensaverActive) {
        screensaverActive = false;
        obdFill(&g_obd, OBD_WHITE, 1);
        /* Redraw static elements */
        obdWriteString(&g_obd, 0, 16, 0, (char *)STR_RENAME_CAR[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);
        obdWriteString(&g_obd, 0, 1, OLED_HEIGHT - HEIGHT8x8, (char *)STR_CONFIRM[g_storedVar.language], FONT_6x8, OBD_WHITE, 1);
        for (uint8_t j = 0; j < 8; j++) {
          obdDrawLine(&g_obd, 80 + j, 16 + j, 80 + j, 30 - j, OBD_BLACK, 1);
        }
        obdDrawLine(&g_obd, 72, 22, 80, 22, OBD_BLACK, 1);
        obdDrawLine(&g_obd, 72, 23, 80, 23, OBD_BLACK, 1);
        obdDrawLine(&g_obd, 72, 24, 80, 24, OBD_BLACK, 1);
        continue;
      }
      /* Exit renameCar routing when CONFIRM is selected */
      if (selectedOption == CAR_NAME_MAX_SIZE - 1)
      {
        /* Change the name of the Car */
        sprintf(g_storedVar.carParam[g_storedVar.selectedCarNumber].carName, "%s", tmpName);
        /* Menu variables are initialized in main loop */
        return;
      }

      /* If in RENAME_CAR_SELECT_OPTION_MODE */
      if (mode == RENAME_CAR_SELECT_OPTION_MODE) {
        /* switch mode */
        mode = RENAME_CAR_SELECT_CHAR_MODE;
        /* Reset encode */
        g_rotaryEncoder.setBoundaries(RENAME_CAR_MIN_ASCII, RENAME_CAR_MAX_ASCII, false);
        g_rotaryEncoder.reset((uint16_t)tmpName[selectedOption]);
        selectedChar = (uint16_t)tmpName[selectedOption];
      }
      /* If in RENAME_CAR_SELECT_CHAR_MODE */
      else {
        /* switch mode */
        mode = RENAME_CAR_SELECT_OPTION_MODE;
        /* Reset encode */
        g_rotaryEncoder.setBoundaries(0, CAR_NAME_MAX_SIZE - 1, false);
        g_rotaryEncoder.reset(selectedOption);
        /* Cancel the upward and downward arrows (draw them black) */
        for (uint8_t j = 0; j < 6; j++) {
          obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 14 - j, 11 - j + (selectedOption * 12), 14 - j, OBD_WHITE, 1);
          obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 33 + j, 11 - j + (selectedOption * 12), 33 + j, OBD_WHITE, 1);
        }
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); return; }
  }
}


/**
 * Show the Throttle Curve selection screen. Shwon when the CURVE item is selected.
 * Draws the throttle curve graph as two segmented lines, that changes when the encoder is rotated (that is, the CURVE parameter is being changed).
 * The calculation for drawing the throttle curve are the same as the ones done in the throttleCurve function.
 * The CURVE parameter and the current trigger value are also displayed.
 */
void showCurveSelection()
{
  uint16_t throttleCurveVertexSpeedRaw;
  uint16_t prevTrigger = g_escVar.outputSpeed_pct;
  uint16_t inputThrottle = (g_storedVar.carParam[g_carSel].throttleCurveVertex.inputThrottle * 100) / THROTTLE_NORMALIZED;  //Take inputThrottle (from 0 to THROTTLE NORMALIZED) and convert it in a 0% to 100% value
  uint16_t minSpeedPctX10 = sensiToPctX10(g_storedVar.carParam[g_carSel].minSpeed);
  uint16_t minSpeedY = map(minSpeedPctX10, 0, 1000, 50, 0);
  uint16_t maxSpeedY = map((uint16_t)g_storedVar.carParam[g_carSel].maxSpeed * 10, 0, 1000, 50, 0);
  
  /*The inputThrottle (X axis) (that ranges from 0 to THROTTLE_NORMALIZED) is converted to the 0-100 range, to simplify calculations.
    The output speed (Y axis) is already expressed in the 0-100 range.
    The origin OLED display screen, which is 128x64, is located on the top-left corner of the screen.
    The origin of the graph is therefore shifted to the pixel (25, 50). Given that the screen heigth is only 64, the y values are also scaled to range from 0 to 50 (they are divided by 2).
    The x pixel values increases from left to right, but the y pixel values increases from top to bottom, therefore every (x, y) value in the (throttle, speed) domain is converted in the pixel domain as:
    x_pixel = (x_throttle + 25);
    y_pixel = (50 - (y_speed / 2)); */
  /* Clear screen and draw x and y axis */
  obdFill(&g_obd, OBD_WHITE, 1);
  obdDrawLine(&g_obd, 25, 0, 25, 50, OBD_BLACK, 1);
  obdDrawLine(&g_obd, 25, 50, 125, 50, OBD_BLACK, 1);
  /* Write the 100%, 0%, 50% and MIN and MAXpoints labels */
  obdWriteString(&g_obd, 0, 0, 0, (char *)"100%", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 58, (char *)"  0%", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 104, 58, (char *)"100%", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, (minSpeedY > 42 ? 42 : minSpeedY), (char *)"MIN", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 28, maxSpeedY, (char *)"MAX", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 64, 58, (char *)"50%", FONT_6x8, OBD_BLACK, 1);

  /* Draw axis ticks at 50%, MIN SPEED and MAX speed points*/
  obdSetPixel(&g_obd, 24, minSpeedY, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 23, minSpeedY, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 25 + inputThrottle, 51, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 25 + inputThrottle, 52, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 26, maxSpeedY, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 27, maxSpeedY, OBD_BLACK, 1);

  /* Set encoder to curve parameters */
  g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
  g_rotaryEncoder.setBoundaries(THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE, false);
  g_rotaryEncoder.reset(g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff);

  /* Save original curve value for cancel */
  uint16_t originalCurveValue = g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
  bool curveCanceled = false;

  /* Calculate the output speed of the throttle curve vertex (as in throttleCurve() function)
     This is calculated as the curveSpeedDiff (from 10% to 90%) percentage of the difference between minSpeed and maxSpeed */
  throttleCurveVertexSpeedRaw = g_storedVar.carParam[g_carSel].minSpeed + ((((uint32_t)g_storedVar.carParam[g_carSel].maxSpeed * SENSI_SCALE) - (uint32_t)g_storedVar.carParam[g_carSel].minSpeed) * ((uint32_t)g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff) / 100);

  /* Draw Line from MIN SPEED, to middle point */
  obdDrawLine(&g_obd, 25, minSpeedY, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), OBD_BLACK, 1);
  /* Draw Line from middle point to 100% */
  obdDrawLine(&g_obd, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), 125, maxSpeedY, OBD_BLACK, 1);

  /* Write the CURVE value */
  sprintf(msgStr, "%3d%c", g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff, '%');
  obdWriteString(&g_obd, 0, OLED_WIDTH - 48, 34, msgStr, FONT_12x16, OBD_BLACK, 1);

  /* Write the trigger value */
  sprintf(msgStr, "%3d%c", prevTrigger, '%');
  obdWriteString(&g_obd, 0, OLED_WIDTH - 32, 26, msgStr, FONT_8x8, OBD_BLACK, 1);

  /* Exit curve function when encoder is clicked */
  while (!g_rotaryEncoder.isEncoderButtonClicked() && !curveCanceled)
  {
    /* Check for brake button press - acts as "cancel" in curve screen */
    static bool brakeBtnInCurve = false;
    static uint32_t lastBrakeBtnCurveTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInCurve && millis() - lastBrakeBtnCurveTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInCurve = true;
        lastBrakeBtnCurveTime = millis();
        /* Cancel changes - restore original value */
        g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff = originalCurveValue;
        curveCanceled = true;
      }
    } else {
      brakeBtnInCurve = false;
    }

    /* Write the trigger value only if it changed */
    if (g_escVar.outputSpeed_pct != prevTrigger)
    {
      /* Update trigger */
      prevTrigger = g_escVar.outputSpeed_pct;

      /* Write the trigger value */
      sprintf(msgStr, "%3d%c", prevTrigger, '%');
      obdWriteString(&g_obd, 0, OLED_WIDTH - 32, 26, msgStr, FONT_8x8, OBD_BLACK, 1);
    }

    /* Get encoder position if it was changed and correct the new lines*/
    if (g_rotaryEncoder.encoderChanged())
    {
      /* Cancel the old lines (draw them in black) */
      obdDrawLine(&g_obd, 25, minSpeedY, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), OBD_WHITE, 1);
      obdDrawLine(&g_obd, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), 125, maxSpeedY, OBD_WHITE, 1);
      
      /* Update the speed coordinate of the vertex */
      g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff = g_rotaryEncoder.readEncoder();
      sprintf(msgStr, "%3d%c", g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff, '%');
      throttleCurveVertexSpeedRaw = g_storedVar.carParam[g_carSel].minSpeed + ((((uint32_t)g_storedVar.carParam[g_carSel].maxSpeed * SENSI_SCALE) - (uint32_t)g_storedVar.carParam[g_carSel].minSpeed) * ((uint32_t)g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff) / 100);
      obdWriteString(&g_obd, 0, OLED_WIDTH - 48, 34, msgStr, FONT_12x16, OBD_BLACK, 1);
      
      /* Draw the new lines */
      obdDrawLine(&g_obd, 25, minSpeedY, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), OBD_BLACK, 1);
      obdDrawLine(&g_obd, 25 + inputThrottle, map((uint16_t)(throttleCurveVertexSpeedRaw * 5), 0, 1000, 50, 0), 125, maxSpeedY, OBD_BLACK, 1);
    }
    else
    {
      /* Service the watchdog, to prevent CPU reset */
      vTaskDelay(10);
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }
  }

  /* Reset encoder */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, getMainMenuItemsCount(), false);
  g_rotaryEncoder.reset(getMainMenuSelector());
  g_escVar.encoderPos = getMainMenuSelector();

  /* Only save if not canceled */
  if (!curveCanceled && !isEscapeToMainRequested()) {
    saveEEPROM(g_storedVar);          /* Save modified values to EEPROM */
  }

  obdFill(&g_obd, OBD_WHITE, 1);    /* Clear screen */

  return;
}
