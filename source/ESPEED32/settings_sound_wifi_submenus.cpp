#include "settings_sound_wifi_submenus.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "ui_text_access.h"
#include "connectivity_portal.h"

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

extern void serviceTimedWiFiPortal();
extern void startTimedWiFiPortal(uint16_t minutes);
extern void stopTimedWiFiPortal();
extern uint16_t getWiFiTimedMinutes();
extern void setWiFiTimedMinutes(uint16_t minutes);

/**
 * Sound settings submenu: BOOT (on/off), RACE (on/off), BACK.
 * BOOT controls startup/calibration/on/off sounds.
 * RACE controls the race mode toggle beep.
 */
void showSoundSettings() {
  const uint8_t SND_ITEMS = SOUND_ITEMS_COUNT;  /* 3: BOOT, RACE, BACK */
  const uint8_t menuFont  = FONT_8x8;
  const uint8_t lineH     = HEIGHT8x8;

  obdFill(&g_obd, OBD_WHITE, 1);

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, SND_ITEMS, false);
  g_rotaryEncoder.reset(1);

  uint8_t sel = 1;
  uint8_t prevSel = 0xFF;
  bool needRedraw = true;

  uint32_t lastInteraction = millis();
  bool ssActive = false;
  uint16_t ssEncoderPos = 0;

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
      sel = (uint8_t)g_rotaryEncoder.readEncoder();
      needRedraw = true;
    }

    /* Encoder button click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == SND_ITEMS) {  /* BACK */
        break;
      } else if (sel == 1) {  /* BOOT toggle */
        g_storedVar.soundBoot = g_storedVar.soundBoot ? 0 : 1;
        saveEEPROM(g_storedVar);
      } else if (sel == 2) {  /* RACE toggle */
        g_storedVar.soundRace = g_storedVar.soundRace ? 0 : 1;
        saveEEPROM(g_storedVar);
      }
      needRedraw = true;
      delay(200);
    }

    /* Brake button = back */
    static bool brakeBtnInSound = false;
    static uint32_t lastBrakeBtnSoundTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInSound && millis() - lastBrakeBtnSoundTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInSound = true;
        lastBrakeBtnSoundTime = millis();
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    } else {
      brakeBtnInSound = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }

    /* Draw */
    if (needRedraw || sel != prevSel) {
      needRedraw = false;
      prevSel = sel;
      uint8_t lang = g_storedVar.language;
      for (uint8_t i = 0; i < SND_ITEMS; i++) {
        bool isSelected = (sel == i + 1);
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)getSoundMenuName(lang, i),
                       menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);
        /* Show ON/OFF value on right for BOOT (i=0) and RACE (i=1) */
        if (i < 2) {
          uint16_t state = (i == 0) ? g_storedVar.soundBoot : g_storedVar.soundRace;
          sprintf(msgStr, "%3s", getOnOffLabel(lang, state));
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

/**
 * WiFi submenu.
 * Items: INFO PAGE, AUTO OFF (minutes), START/STOP WIFI, BACK.
 * MINUTES is a runtime-only value used for timed background activation.
 */
void showWiFiSettings() {
  uint16_t lang = g_storedVar.language;

  const char* lblOpen[4]    = {"INFO SIDE",   "INFO PAGE", "INFO PAGE", "INFO PAGE"};
  const char* lblTimer[4]   = {"AUTO AV",     "AUTO OFF",  "AUTO OFF",  "AUTO OFF"};
  const char* lblStartBg[4] = {"START WIFI", "START WIFI", "START WIFI", "START WIFI"};
  const char* lblStopBg[4]  = {"STOPP WIFI", "STOP WIFI",  "STOP WIFI",  "STOP WIFI"};

  const uint8_t NUM_ITEMS = 4;  /* 0=NOW, 1=MINUTES, 2=ACTIVE, 3=BACK */
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH = HEIGHT8x8;

  int8_t sel = 0;
  bool editing = false;
  uint16_t tmpMinutes = constrain(getWiFiTimedMinutes(), 1, 120);
  bool needRedraw = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  while (true) {
    serviceTimedWiFiPortal();

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
        tmpMinutes = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (int8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        setWiFiTimedMinutes(tmpMinutes);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(sel);
      } else if (sel == 0) {
        showWiFiPortalScreen();
        lastInteraction = millis();
        screensaverActive = false;
      } else if (sel == 1) {
        editing = true;
        tmpMinutes = constrain(getWiFiTimedMinutes(), 1, 120);
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, 120, false);
        g_rotaryEncoder.reset(tmpMinutes);
      } else if (sel == 2) {
        if (isWiFiPortalActive()) {
          stopTimedWiFiPortal();
        } else {
          startTimedWiFiPortal(getWiFiTimedMinutes());
        }
      } else {
        break;
      }
      needRedraw = true;
      delay(120);
    }

    static bool brakeBtnInWifi = false;
    static uint32_t lastBrakeBtnWifiTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (!brakeBtnInWifi && millis() - lastBrakeBtnWifiTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInWifi = true;
        lastBrakeBtnWifiTime = millis();
        if (editing) {
          editing = false;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
          g_rotaryEncoder.reset(sel);
          needRedraw = true;
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeBtnInWifi = false;
    }

    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);

      bool s0 = (!editing && sel == 0);
      obdWriteString(&g_obd, 0, 0, 0 * lineH, (char*)lblOpen[lang], menuFont, s0 ? OBD_WHITE : OBD_BLACK, 1);

      bool s1 = (!editing && sel == 1);
      obdWriteString(&g_obd, 0, 0, 1 * lineH, (char*)lblTimer[lang], menuFont, s1 ? OBD_WHITE : OBD_BLACK, 1);
      char minutesStr[10];
      uint16_t shownMinutes = editing ? tmpMinutes : constrain(getWiFiTimedMinutes(), 1, 120);
      snprintf(minutesStr, sizeof(minutesStr), "%3dm", shownMinutes);
      uint8_t mx = OLED_WIDTH - (uint8_t)(strlen(minutesStr) * WIDTH8x8);
      obdWriteString(&g_obd, 0, mx, 1 * lineH, minutesStr, menuFont, (s1 || editing) ? OBD_WHITE : OBD_BLACK, 1);

      bool s2 = (!editing && sel == 2);
      const char* actionStr = isWiFiPortalActive() ? lblStopBg[lang] : lblStartBg[lang];
      obdWriteString(&g_obd, 0, 0, 2 * lineH, (char*)actionStr, menuFont, s2 ? OBD_WHITE : OBD_BLACK, 1);

      bool s3 = (!editing && sel == 3);
      obdWriteString(&g_obd, 0, 0, 3 * lineH, (char*)getBackLabel(lang), menuFont, s3 ? OBD_WHITE : OBD_BLACK, 1);

      needRedraw = false;
    }

    if (checkRaceModeEscape()) { requestEscapeToMain(); break; }
    vTaskDelay(10);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
