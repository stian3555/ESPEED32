#include "wifi_backup.h"
#include "slot_ESC.h"
#include "HAL.h"
#include <Update.h>

/* External references from main .ino */
extern StoredVar_type g_storedVar;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern void saveEEPROM(StoredVar_type toSave);

/* Web server instance (heap-allocated only when active) */
static WebServer* g_wifiServer = nullptr;
static String g_uploadBuffer;
static char g_wifiSuffix[5] = "";  /* MAC-based suffix, e.g. "A3B4" */

/* HTML page served to browser */
static const char WIFI_HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESPEED32</title>
<style>
body{font-family:Arial,sans-serif;max-width:480px;margin:20px auto;padding:0 15px;background:#1a1a2e;color:#eee}
h1{color:#e94560;text-align:center;margin-bottom:5px}
p.sub{text-align:center;color:#888;margin-top:0}
h2{color:#ccc;font-size:16px;margin-bottom:5px}
.btn{display:block;width:100%;padding:15px;margin:10px 0;border:none;border-radius:8px;font-size:16px;cursor:pointer;box-sizing:border-box}
.dl{background:#0f3460;color:#fff}
.dl:hover{background:#1a4a80}
.ul{background:#e94560;color:#fff}
.ul:hover{background:#ff6b81}
.ota{background:#ff8c00;color:#000}
.ota:hover{background:#ffa500}
hr{border-color:#333;margin:20px 0}
input[type=file]{display:block;margin:10px 0;color:#eee;width:100%}
.st{margin:15px 0;padding:12px;border-radius:5px;display:none;text-align:center}
.ok{background:#16c79a;color:#000}
.err{background:#e94560;color:#fff}
.warn{color:#ff8c00;font-size:13px;text-align:center}
</style>
</head>
<body>
<h1>ESPEED32</h1>
<p class="sub">v<span id="ver">%VERSION%</span> &middot; <span id="devid">%SUFFIX%</span></p>
<h2>Config Backup &amp; Restore (.json)</h2>
<a id="dl" href="/backup"><button class="btn dl">Download Config Backup</button></a>
<form id="uf">
<input type="file" id="fi" accept=".json">
<button type="submit" class="btn ul">Restore Config</button>
</form>
<div id="status" class="st"></div>
<hr>
<h2>Firmware Update (.bin)</h2>
<form id="of">
<input type="file" id="fw" accept=".bin">
<button type="submit" class="btn ota">Upload Firmware</button>
</form>
<p class="warn">Do not disconnect power during update!</p>
<div id="ostatus" class="st"></div>
<script>
document.getElementById('dl').onclick=function(){
  var d=new Date().toISOString().slice(0,10);
  var id=document.getElementById('devid').textContent;
  var v=document.getElementById('ver').textContent;
  this.download=d+'-espeed32_v'+v+'_'+id+'_backup.json';
};
document.getElementById('uf').onsubmit=function(e){
  e.preventDefault();
  var f=document.getElementById('fi').files[0];
  if(!f){alert('Select a JSON file first');return}
  if(!f.name.toLowerCase().endsWith('.json')){alert('Only .json files allowed for config restore');return}
  var fd=new FormData();fd.append('file',f);
  var s=document.getElementById('status');
  s.style.display='block';s.className='st';s.textContent='Uploading...';
  fetch('/restore',{method:'POST',body:fd}).then(function(r){return r.text()}).then(function(t){
    s.className='st '+(t.indexOf('OK')===0?'ok':'err');s.textContent=t;
  }).catch(function(err){s.className='st err';s.textContent='Error: '+err});
};
document.getElementById('of').onsubmit=function(e){
  e.preventDefault();
  var f=document.getElementById('fw').files[0];
  if(!f){alert('Select a .bin firmware file first');return}
  if(!f.name.toLowerCase().endsWith('.bin')){alert('Only .bin files allowed for firmware update');return}
  var fd=new FormData();fd.append('file',f);
  var s=document.getElementById('ostatus');
  s.style.display='block';s.className='st';s.textContent='Uploading firmware...';
  fetch('/ota',{method:'POST',body:fd}).then(function(r){return r.text()}).then(function(t){
    s.className='st '+(t.indexOf('OK')===0?'ok':'err');s.textContent=t;
  }).catch(function(err){s.className='st err';s.textContent='Error: '+err});
};
</script>
</body>
</html>
)rawliteral";


/**
 * @brief Build JSON backup string from stored variables
 */
static String buildJsonBackup() {
  String json;
  json.reserve(5200);
  char buf[128];

  json += "{\n";
  sprintf(buf, "  \"version\": %d,\n", STORED_VAR_VERSION);              json += buf;
  sprintf(buf, "  \"selectedCarNumber\": %u,\n", g_storedVar.selectedCarNumber); json += buf;
  sprintf(buf, "  \"minTrigger_raw\": %d,\n", g_storedVar.minTrigger_raw); json += buf;
  sprintf(buf, "  \"maxTrigger_raw\": %d,\n", g_storedVar.maxTrigger_raw); json += buf;
  sprintf(buf, "  \"viewMode\": %u,\n", g_storedVar.viewMode);           json += buf;
  sprintf(buf, "  \"screensaverTimeout\": %u,\n", g_storedVar.screensaverTimeout); json += buf;
  sprintf(buf, "  \"soundMode\": %u,\n", g_storedVar.soundMode);         json += buf;
  sprintf(buf, "  \"gridCarSelectEnabled\": %u,\n", g_storedVar.gridCarSelectEnabled); json += buf;
  sprintf(buf, "  \"raceViewMode\": %u,\n", g_storedVar.raceViewMode);   json += buf;
  sprintf(buf, "  \"language\": %u,\n", g_storedVar.language);           json += buf;
  sprintf(buf, "  \"textCase\": %u,\n", g_storedVar.textCase);           json += buf;
  sprintf(buf, "  \"listFontSize\": %u,\n", g_storedVar.listFontSize);   json += buf;
  sprintf(buf, "  \"startupDelay\": %u,\n", g_storedVar.startupDelay);   json += buf;
  sprintf(buf, "  \"screensaverLine1\": \"%s\",\n", g_storedVar.screensaverLine1); json += buf;
  sprintf(buf, "  \"screensaverLine2\": \"%s\",\n", g_storedVar.screensaverLine2); json += buf;

  json += "  \"cars\": [\n";
  for (int i = 0; i < CAR_MAX_COUNT; i++) {
    const CarParam_type& c = g_storedVar.carParam[i];
    json += "    {\n";
    sprintf(buf, "      \"name\": \"%s\",\n", c.carName);                json += buf;
    sprintf(buf, "      \"minSpeed\": %u,\n", c.minSpeed);               json += buf;
    sprintf(buf, "      \"brake\": %u,\n", c.brake);                     json += buf;
    sprintf(buf, "      \"dragBrake\": %u,\n", c.dragBrake);             json += buf;
    sprintf(buf, "      \"maxSpeed\": %u,\n", c.maxSpeed);               json += buf;
    sprintf(buf, "      \"curveInput\": %u,\n", c.throttleCurveVertex.inputThrottle); json += buf;
    sprintf(buf, "      \"curveDiff\": %u,\n", c.throttleCurveVertex.curveSpeedDiff); json += buf;
    sprintf(buf, "      \"antiSpin\": %u,\n", c.antiSpin);               json += buf;
    sprintf(buf, "      \"freqPWM\": %u,\n", c.freqPWM);                 json += buf;
    sprintf(buf, "      \"brakeButton\": %u\n", c.brakeButtonReduction);  json += buf;
    json += "    }";
    if (i < CAR_MAX_COUNT - 1) json += ",";
    json += "\n";
  }
  json += "  ]\n}\n";
  return json;
}


/**
 * @brief Parse an integer value from JSON by key name
 */
static bool parseJsonInt(const String& json, const char* key, int32_t& outVal) {
  String search = String("\"") + key + "\"";
  int idx = json.indexOf(search);
  if (idx < 0) return false;
  int colon = json.indexOf(':', idx);
  if (colon < 0) return false;
  int start = colon + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\n' || json[start] == '\r')) start++;
  int end = start;
  while (end < (int)json.length() && (isDigit(json[end]) || json[end] == '-')) end++;
  if (end == start) return false;
  outVal = json.substring(start, end).toInt();
  return true;
}


/**
 * @brief Parse a string value from JSON by key name
 */
static bool parseJsonStr(const String& json, const char* key, char* outStr, int maxLen) {
  String search = String("\"") + key + "\"";
  int idx = json.indexOf(search);
  if (idx < 0) return false;
  int colon = json.indexOf(':', idx);
  if (colon < 0) return false;
  int qStart = json.indexOf('"', colon + 1);
  if (qStart < 0) return false;
  int qEnd = json.indexOf('"', qStart + 1);
  if (qEnd < 0) return false;
  int len = qEnd - qStart - 1;
  if (len >= maxLen) len = maxLen - 1;
  json.substring(qStart + 1, qStart + 1 + len).toCharArray(outStr, maxLen);
  return true;
}


/**
 * @brief Validate integer is within range
 */
static bool inRange(int32_t val, int32_t minVal, int32_t maxVal) {
  return val >= minVal && val <= maxVal;
}


/**
 * @brief Parse and validate uploaded JSON, populate temporary StoredVar
 * @return true if valid, false with error message
 */
static bool parseAndValidateJson(const String& json, StoredVar_type* sv, String* errorMsg) {
  int32_t v;

  /* Initialize with current settings as defaults (missing fields keep current values) */
  *sv = g_storedVar;

  /* Version check - warn but allow cross-version restore for car profiles */
  if (parseJsonInt(json, "version", v) && v != STORED_VAR_VERSION) {
    /* Different version: car profiles are restored, global settings use defaults for missing fields */
  }

  /* Parse global settings - use value if present and valid, otherwise keep default */
  if (parseJsonInt(json, "selectedCarNumber", v) && inRange(v, 0, CAR_MAX_COUNT - 1))
    sv->selectedCarNumber = v;
  if (parseJsonInt(json, "minTrigger_raw", v))
    sv->minTrigger_raw = (int16_t)v;
  if (parseJsonInt(json, "maxTrigger_raw", v))
    sv->maxTrigger_raw = (int16_t)v;
  if (parseJsonInt(json, "viewMode", v) && inRange(v, 0, 1))
    sv->viewMode = v;
  if (parseJsonInt(json, "screensaverTimeout", v) && inRange(v, 0, SCREENSAVER_TIMEOUT_MAX))
    sv->screensaverTimeout = v;
  if (parseJsonInt(json, "soundMode", v) && inRange(v, SOUND_MODE_OFF, SOUND_MODE_ALL))
    sv->soundMode = v;
  if (parseJsonInt(json, "gridCarSelectEnabled", v) && inRange(v, 0, 1))
    sv->gridCarSelectEnabled = v;
  if (parseJsonInt(json, "raceViewMode", v) && inRange(v, 0, 2))
    sv->raceViewMode = v;
  if (parseJsonInt(json, "language", v) && inRange(v, LANG_NOR, LANG_ACD))
    sv->language = v;
  if (parseJsonInt(json, "textCase", v) && inRange(v, TEXT_CASE_UPPER, TEXT_CASE_PASCAL))
    sv->textCase = v;
  if (parseJsonInt(json, "listFontSize", v) && inRange(v, FONT_SIZE_LARGE, FONT_SIZE_SMALL))
    sv->listFontSize = v;
  if (parseJsonInt(json, "startupDelay", v) && inRange(v, STARTUP_DELAY_MIN, STARTUP_DELAY_MAX))
    sv->startupDelay = v;

  /* Parse screensaver text fields */
  char tempStr[SCREENSAVER_TEXT_MAX];
  if (parseJsonStr(json, "screensaverLine1", tempStr, SCREENSAVER_TEXT_MAX))
    strncpy(sv->screensaverLine1, tempStr, SCREENSAVER_TEXT_MAX);
  if (parseJsonStr(json, "screensaverLine2", tempStr, SCREENSAVER_TEXT_MAX))
    strncpy(sv->screensaverLine2, tempStr, SCREENSAVER_TEXT_MAX);

  /* Parse car profiles */
  int carStart = json.indexOf("\"cars\"");
  if (carStart < 0) { *errorMsg = "Error: missing cars array"; return false; }

  int searchPos = json.indexOf('[', carStart);
  if (searchPos < 0) { *errorMsg = "Error: malformed cars array"; return false; }

  for (int i = 0; i < CAR_MAX_COUNT; i++) {
    /* Find next car object */
    int objStart = json.indexOf('{', searchPos);
    int objEnd = json.indexOf('}', objStart);
    if (objStart < 0 || objEnd < 0) {
      *errorMsg = "Error: missing car profile " + String(i);
      return false;
    }
    String carJson = json.substring(objStart, objEnd + 1);
    searchPos = objEnd + 1;

    CarParam_type& c = sv->carParam[i];

    /* Name - use temp buffer to preserve names up to CAR_NAME_MAX_SIZE chars */
    char tempName[16];
    if (!parseJsonStr(carJson, "name", tempName, sizeof(tempName))) {
      *errorMsg = "Error: missing name in car " + String(i); return false;
    }
    memset(c.carName, 0, CAR_NAME_MAX_SIZE);
    strncpy(c.carName, tempName, CAR_NAME_MAX_SIZE);
    c.carNumber = i;

    /* Numeric fields */
    if (!parseJsonInt(carJson, "minSpeed", v) || !inRange(v, 0, MIN_SPEED_MAX_VALUE)) {
      *errorMsg = "Error: invalid minSpeed in car " + String(i); return false;
    }
    c.minSpeed = v;

    if (!parseJsonInt(carJson, "brake", v) || !inRange(v, 0, BRAKE_MAX_VALUE)) {
      *errorMsg = "Error: invalid brake in car " + String(i); return false;
    }
    c.brake = v;

    if (!parseJsonInt(carJson, "dragBrake", v) || !inRange(v, 0, DRAG_MAX_VALUE)) {
      *errorMsg = "Error: invalid dragBrake in car " + String(i); return false;
    }
    c.dragBrake = v;

    if (!parseJsonInt(carJson, "maxSpeed", v) || !inRange(v, 5, 100)) {
      *errorMsg = "Error: invalid maxSpeed in car " + String(i); return false;
    }
    c.maxSpeed = v;

    if (!parseJsonInt(carJson, "curveInput", v) || !inRange(v, 0, THROTTLE_NORMALIZED)) {
      *errorMsg = "Error: invalid curveInput in car " + String(i); return false;
    }
    c.throttleCurveVertex.inputThrottle = v;

    if (!parseJsonInt(carJson, "curveDiff", v) || !inRange(v, THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE)) {
      *errorMsg = "Error: invalid curveDiff in car " + String(i); return false;
    }
    c.throttleCurveVertex.curveSpeedDiff = v;

    if (!parseJsonInt(carJson, "antiSpin", v) || !inRange(v, 0, ANTISPIN_MAX_VALUE)) {
      *errorMsg = "Error: invalid antiSpin in car " + String(i); return false;
    }
    c.antiSpin = v;

    if (!parseJsonInt(carJson, "freqPWM", v) || !inRange(v, FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100)) {
      *errorMsg = "Error: invalid freqPWM in car " + String(i); return false;
    }
    c.freqPWM = v;

    if (!parseJsonInt(carJson, "brakeButton", v) || !inRange(v, 0, 100)) {
      *errorMsg = "Error: invalid brakeButton in car " + String(i); return false;
    }
    c.brakeButtonReduction = v;
  }

  return true;
}


/* OTA progress tracking */
static size_t g_otaTotal = 0;
static size_t g_otaWritten = 0;
static bool g_otaInProgress = false;

/* HTTP route handlers */

static void handleRoot() {
  String page = FPSTR(WIFI_HTML_PAGE);
  page.replace("%SUFFIX%", g_wifiSuffix);
  char ver[8];
  sprintf(ver, "%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);
  page.replace("%VERSION%", ver);
  g_wifiServer->send(200, "text/html", page);
}

static void handleBackup() {
  String json = buildJsonBackup();
  g_wifiServer->send(200, "application/json", json);
}

static void handleRestoreUpload() {
  HTTPUpload& upload = g_wifiServer->upload();
  if (upload.status == UPLOAD_FILE_START) {
    g_uploadBuffer = "";
    g_uploadBuffer.reserve(6144);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    g_uploadBuffer += String((char*)upload.buf, upload.currentSize);
  }
}

static void handleRestore() {
  if (g_uploadBuffer.length() == 0) {
    g_wifiServer->send(400, "text/plain", "Error: no file uploaded");
    return;
  }

  StoredVar_type tempVar;
  String errorMsg;

  if (parseAndValidateJson(g_uploadBuffer, &tempVar, &errorMsg)) {
    g_storedVar = tempVar;
    saveEEPROM(g_storedVar);
    g_wifiServer->send(200, "text/plain", "OK - Settings restored! Restarting...");
    g_uploadBuffer = "";
    delay(1000);
    ESP.restart();
  } else {
    g_wifiServer->send(400, "text/plain", errorMsg);
  }
  g_uploadBuffer = "";
}


/**
 * @brief Handle OTA firmware upload - streams .bin directly to flash
 */
static void handleOtaUpload() {
  HTTPUpload& upload = g_wifiServer->upload();
  char msgStr[32];

  if (upload.status == UPLOAD_FILE_START) {
    g_otaTotal = upload.totalSize;  /* May be 0 if browser doesn't send content-length */
    g_otaWritten = 0;
    g_otaInProgress = true;

    /* Show update in progress on OLED */
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 16, 0, (char*)"OTA Update", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Updating...", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, (char*)"Do not power off!", FONT_6x8, OBD_BLACK, 1);

    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      g_otaInProgress = false;
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) == upload.currentSize) {
      g_otaWritten += upload.currentSize;
      /* Update progress on OLED */
      sprintf(msgStr, "%u KB", (unsigned int)(g_otaWritten / 1024));
      obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, msgStr, FONT_8x8, OBD_BLACK, 1);
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      obdFill(&g_obd, OBD_WHITE, 1);
      obdWriteString(&g_obd, 0, 8, 24, (char*)"OTA OK!", FONT_12x16, OBD_BLACK, 1);
    } else {
      obdFill(&g_obd, OBD_WHITE, 1);
      obdWriteString(&g_obd, 0, 0, 24, (char*)"OTA FAIL!", FONT_12x16, OBD_BLACK, 1);
    }
    g_otaInProgress = false;
  }
}

/**
 * @brief Handle OTA completion response - called after upload finishes
 */
static void handleOta() {
  if (Update.hasError()) {
    g_wifiServer->send(400, "text/plain", "Error: firmware update failed");
  } else {
    g_wifiServer->send(200, "text/plain", "OK - Firmware updated! Restarting...");
    delay(1000);
    ESP.restart();
  }
}


/**
 * @brief Main WiFi backup/restore screen - called from settings menu
 * @details Starts WiFi AP, runs HTTP server, shows info on OLED.
 *          Returns when user presses encoder button or brake button.
 */
void showWiFiBackupScreen() {
  char msgStr[32];

  /* Build unique SSID from chip MAC address (last 2 bytes) */
  char ssid[20];
  uint64_t mac = ESP.getEfuseMac();
  sprintf(g_wifiSuffix, "%02X%02X", (uint8_t)(mac >> 8), (uint8_t)(mac));
  sprintf(ssid, "%s_%s", WIFI_SSID, g_wifiSuffix);

  /* Start WiFi AP with WPA2 */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, WIFI_PASS, WIFI_AP_CHANNEL, 0, WIFI_MAX_CONNECTIONS);
  delay(100);
  IPAddress ip = WiFi.softAPIP();

  /* Start web server */
  g_wifiServer = new WebServer(80);
  g_wifiServer->on("/", handleRoot);
  g_wifiServer->on("/backup", HTTP_GET, handleBackup);
  g_wifiServer->on("/restore", HTTP_POST, handleRestore, handleRestoreUpload);
  g_wifiServer->on("/ota", HTTP_POST, handleOta, handleOtaUpload);
  g_wifiServer->begin();

  /* Display WiFi info on OLED (FONT_6x8 = 21 chars/line) */
  obdFill(&g_obd, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, 40, 0, (char*)"WiFi", FONT_8x8, OBD_BLACK, 1);

  sprintf(msgStr, "SSID: %s", ssid);
  obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Pass: %s", WIFI_PASS);
  obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);

  obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, (char*)"Open in browser:", FONT_6x8, OBD_BLACK, 1);
  sprintf(msgStr, "%s", ip.toString().c_str());
  obdWriteString(&g_obd, 0, 0, 7 * HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);

  /* Service loop - handle HTTP requests until user exits */
  while (true) {
    g_wifiServer->handleClient();

    /* Block exit during OTA to prevent bricking */
    if (!g_otaInProgress) {
      if (g_rotaryEncoder.isEncoderButtonClicked()) {
        break;
      }

      if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
        break;
      }
    }

    vTaskDelay(1);
  }

  /* Cleanup */
  g_wifiServer->stop();
  delete g_wifiServer;
  g_wifiServer = nullptr;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  obdFill(&g_obd, OBD_WHITE, 1);
}
