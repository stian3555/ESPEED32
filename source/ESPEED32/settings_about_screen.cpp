#include "settings_about_screen.h"
#include <Arduino.h>
#include <esp_mac.h>
#include "slot_ESC.h"
#include "connectivity_portal.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;

extern bool consumeScreensaverWakeInput(bool wakeTriggered);
extern bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive);
extern void showScreensaver();

/**
 * Show scrollable device information screen.
 * Includes firmware/data versions, chip details, WiFi/BT MAC addresses and build info.
 */
void showAboutScreen() {
  static const uint8_t ABOUT_MAX_LINES = 18;
  static const uint8_t ABOUT_VISIBLE_LINES = 7;  /* rows below title */
  char lines[ABOUT_MAX_LINES][22];
  uint8_t lineCount = 0;
  char line[40];

  auto addLine = [&](const char* txt) {
    if (lineCount >= ABOUT_MAX_LINES || txt == nullptr) return;
    strncpy(lines[lineCount], txt, 21);
    lines[lineCount][21] = '\0';
    lineCount++;
  };

  uint8_t wifiMac[6] = {0};
  uint8_t btMac[6] = {0};
  bool wifiOk = (esp_read_mac(wifiMac, ESP_MAC_WIFI_STA) == ESP_OK);
  bool btOk = (esp_read_mac(btMac, ESP_MAC_BT) == ESP_OK);

  snprintf(line, sizeof(line), "FW: v%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);
  addLine(line);
  snprintf(line, sizeof(line), "Data: v%d", STORED_VAR_VERSION);
  addLine(line);
  addLine("Sensor:");
  HAL_GetTriggerSensorInfo(line, sizeof(line));
  addLine(line);
  if (wifiOk) {
    snprintf(line, sizeof(line), "ID: %02X%02X", wifiMac[4], wifiMac[5]);
  } else {
    uint64_t mac = ESP.getEfuseMac();
    snprintf(line, sizeof(line), "ID: %02X%02X", (uint8_t)(mac >> 8), (uint8_t)(mac));
  }
  addLine(line);

  String chipModel = String(ESP.getChipModel());
  snprintf(line, sizeof(line), "Chip: %s", chipModel.c_str());
  addLine(line);
  snprintf(line, sizeof(line), "Rev:%d Cores:%d", ESP.getChipRevision(), ESP.getChipCores());
  addLine(line);
  snprintf(line, sizeof(line), "Flash: %uMB", (unsigned int)(ESP.getFlashChipSize() / (1024UL * 1024UL)));
  addLine(line);
  snprintf(line, sizeof(line), "Heap free: %uKB", (unsigned int)(ESP.getFreeHeap() / 1024UL));
  addLine(line);

  addLine("WiFi MAC:");
  if (wifiOk) {
    snprintf(line, sizeof(line), "%02X:%02X:%02X:%02X:%02X:%02X",
             wifiMac[0], wifiMac[1], wifiMac[2], wifiMac[3], wifiMac[4], wifiMac[5]);
    addLine(line);
  } else {
    addLine("unavailable");
  }

  addLine("BT MAC:");
  if (btOk) {
    snprintf(line, sizeof(line), "%02X:%02X:%02X:%02X:%02X:%02X",
             btMac[0], btMac[1], btMac[2], btMac[3], btMac[4], btMac[5]);
    addLine(line);
  } else {
    addLine("unavailable");
  }

  snprintf(line, sizeof(line), "Built: %s", __DATE__);
  addLine(line);
  snprintf(line, sizeof(line), "Time: %s", __TIME__);
  addLine(line);

  uint16_t maxScroll = (lineCount > ABOUT_VISIBLE_LINES) ? (lineCount - ABOUT_VISIBLE_LINES) : 0;
  uint16_t scroll = 0;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, maxScroll, false);
  g_rotaryEncoder.reset(0);

  auto drawAbout = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, centerX8x8("About"), 0, (char*)"About", FONT_8x8, OBD_BLACK, 1);

    if (maxScroll > 0) {
      snprintf(line, sizeof(line), "%u/%u", (unsigned int)(scroll + 1), (unsigned int)(maxScroll + 1));
      int sx = OLED_WIDTH - ((int)strlen(line) * 6);
      if (sx < 0) sx = 0;
      obdWriteString(&g_obd, 0, sx, 0, line, FONT_6x8, OBD_BLACK, 1);
    }

    for (uint8_t i = 0; i < ABOUT_VISIBLE_LINES; i++) {
      uint16_t idx = scroll + i;
      if (idx >= lineCount) break;
      obdWriteString(&g_obd, 0, 0, (i + 1) * HEIGHT8x8, lines[idx], FONT_6x8, OBD_BLACK, 1);
    }
  };

  drawAbout();

  /* Easter egg */
  delay(400);
  sound(NOTE_A, 100); delay(80);
  sound(NOTE_A, 350); delay(80);
  sound(NOTE_A, 100);

  bool aboutBtnHeld = false;
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
        drawAbout();
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
          drawAbout();
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        drawAbout();
      }
      delay(10);
      continue;
    }

    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      uint16_t newScroll = g_rotaryEncoder.readEncoder();
      if (newScroll != scroll) {
        scroll = newScroll;
        drawAbout();
      }
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      break;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (!aboutBtnHeld) {
        aboutBtnHeld = true;
      }
    } else {
      if (aboutBtnHeld) break;
    }

    vTaskDelay(1);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
