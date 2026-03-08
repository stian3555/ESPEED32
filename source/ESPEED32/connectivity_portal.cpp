#include "connectivity_portal.h"
#include "slot_ESC.h"
#include "HAL.h"
#include <FS.h>
#include <SPIFFS.h>
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
static bool g_spiffsMounted = false;

/* Minimal fallback page if /ui/index.html is missing on SPIFFS */
static const char UI_FALLBACK_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESPEED32 Recovery</title>
</head>
<body style="font-family:Arial,sans-serif;max-width:560px;margin:20px auto;padding:0 12px;">
<h1>ESPEED32 Recovery Page</h1>
<p>Missing SPIFFS file: <code>/ui/index.html</code></p>
<p>This page is intentionally minimal.</p>
<p><a href="/backup">Download Config Backup (.json)</a></p>
<form method="POST" action="/ota" enctype="multipart/form-data">
  <p><b>Firmware Recovery (.bin)</b></p>
  <input type="file" name="file" accept=".bin" required>
  <button type="submit">Upload Firmware</button>
</form>
<p><b>Restore full web UI:</b></p>
<ol>
  <li>Ensure <code>source/ESPEED32/data/ui/index.html</code> exists.</li>
  <li>Upload SPIFFS image to the controller.</li>
  <li>Reload <code>/</code>.</li>
</ol>
</body>
</html>
)rawliteral";

/* Fallback page shown if docs are missing from SPIFFS */
static const char DOCS_MISSING_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESPEED32 Docs Not Found</title>
<style>
body{font-family:Arial,sans-serif;max-width:560px;margin:20px auto;padding:0 15px;background:#1a1a2e;color:#eee}
h1{color:#e94560;margin-bottom:8px}
code{background:#111;padding:2px 6px;border-radius:4px}
li{margin:8px 0}
a{color:#7ed6ff}
</style>
</head>
<body>
<h1>Documentation Not Found</h1>
<p>Could not open <code>%PATH%</code> from SPIFFS.</p>
<p>To update docs on the controller:</p>
<ol>
<li>Edit files in <code>source/ESPEED32/data/docs/</code> (Git source of truth).</li>
<li>Upload the filesystem image from your IDE (SPIFFS upload).</li>
<li>Refresh this page.</li>
</ol>
<p><a href="/">Back to backup/restore page</a></p>
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
  sprintf(buf, "  \"soundBoot\": %u,\n", g_storedVar.soundBoot);         json += buf;
  sprintf(buf, "  \"soundRace\": %u,\n", g_storedVar.soundRace);         json += buf;
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
    sprintf(buf, "      \"maxSpeed\": %u,\n", c.maxSpeed);               json += buf;
    sprintf(buf, "      \"curveInput\": %u,\n", c.throttleCurveVertex.inputThrottle); json += buf;
    sprintf(buf, "      \"curveDiff\": %u,\n", c.throttleCurveVertex.curveSpeedDiff); json += buf;
    sprintf(buf, "      \"antiSpin\": %u,\n", c.antiSpin);               json += buf;
    sprintf(buf, "      \"freqPWM\": %u,\n", c.freqPWM);                 json += buf;
    sprintf(buf, "      \"brakeButton\": %u,\n", c.brakeButtonReduction);  json += buf;
    sprintf(buf, "      \"quickBrakeEnabled\": %u,\n", c.quickBrakeEnabled);   json += buf;
    sprintf(buf, "      \"quickBrakeThreshold\": %u,\n", c.quickBrakeThreshold); json += buf;
    sprintf(buf, "      \"quickBrakeStrength\": %u\n", c.quickBrakeStrength);  json += buf;
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
  if (parseJsonInt(json, "soundBoot", v) && inRange(v, 0, 1))
    sv->soundBoot = v;
  if (parseJsonInt(json, "soundRace", v) && inRange(v, 0, 1))
    sv->soundRace = v;
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
      break;  /* fewer profiles in backup than CAR_MAX_COUNT — keep defaults for the rest */
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

    /* Quick brake fields — optional for backwards compatibility with older backups */
    if (parseJsonInt(carJson, "quickBrakeEnabled", v) && inRange(v, 0, 1))
      c.quickBrakeEnabled = v;
    if (parseJsonInt(carJson, "quickBrakeThreshold", v) && inRange(v, 0, QUICK_BRAKE_THRESHOLD_MAX))
      c.quickBrakeThreshold = v;
    if (parseJsonInt(carJson, "quickBrakeStrength", v) && inRange(v, 0, QUICK_BRAKE_STRENGTH_MAX))
      c.quickBrakeStrength = v;
  }

  return true;
}

static void appendJsonEscaped(String& out, const char* in) {
  if (in == nullptr) return;
  while (*in) {
    char c = *in++;
    if (c == '\"' || c == '\\') {
      out += '\\';
      out += c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else {
      out += c;
    }
  }
}

static void appendSchemaIntField(
  String& out, bool& first, const char* id, const char* label,
  int32_t minVal, int32_t maxVal, int32_t step, const char* unit = nullptr) {
  if (!first) out += ",";
  first = false;
  char buf[256];
  snprintf(buf, sizeof(buf),
           "{\"id\":\"%s\",\"label\":\"%s\",\"type\":\"int\",\"min\":%ld,\"max\":%ld,\"step\":%ld",
           id, label, (long)minVal, (long)maxVal, (long)step);
  out += buf;
  if (unit != nullptr && unit[0] != '\0') {
    out += ",\"unit\":\"";
    out += unit;
    out += "\"";
  }
  out += "}";
}

static void appendSchemaStringField(
  String& out, bool& first, const char* id, const char* label, int32_t maxLen) {
  if (!first) out += ",";
  first = false;
  char buf[200];
  snprintf(buf, sizeof(buf),
           "{\"id\":\"%s\",\"label\":\"%s\",\"type\":\"string\",\"maxLen\":%ld}",
           id, label, (long)maxLen);
  out += buf;
}

static void appendSchemaEnumField(
  String& out, bool& first, const char* id, const char* label, const char* optionsJson) {
  if (!first) out += ",";
  first = false;
  out += "{\"id\":\"";
  out += id;
  out += "\",\"label\":\"";
  out += label;
  out += "\",\"type\":\"enum\",\"options\":";
  out += optionsJson;
  out += "}";
}

static String buildSchemaJson() {
  String json;
  json.reserve(5000);

  char buf[64];
  json += "{";
  snprintf(buf, sizeof(buf), "\"fwVersion\":\"%d.%d\",", SW_MAJOR_VERSION, SW_MINOR_VERSION);
  json += buf;
  snprintf(buf, sizeof(buf), "\"carCount\":%d,", CAR_MAX_COUNT);
  json += buf;

  json += "\"global\":[";
  bool first = true;
  appendSchemaIntField(json, first, "selectedCarNumber", "Active Car", 0, CAR_MAX_COUNT - 1, 1);
  appendSchemaEnumField(json, first, "viewMode", "View Mode",
                        "[{\"value\":0,\"label\":\"LIST\"},{\"value\":1,\"label\":\"GRID\"}]");
  appendSchemaIntField(json, first, "screensaverTimeout", "Screensaver Timeout", 0, SCREENSAVER_TIMEOUT_MAX, 1, "s");
  appendSchemaIntField(json, first, "powerSaveTimeout", "Sleep Timeout", 0, POWER_SAVE_TIMEOUT_MAX, 1, "min");
  appendSchemaIntField(json, first, "deepSleepTimeout", "Deep Sleep Timeout", 0, DEEP_SLEEP_TIMEOUT_MAX, 1, "min");
  appendSchemaEnumField(json, first, "soundBoot", "Boot Sound",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaEnumField(json, first, "soundRace", "Race Sound",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaEnumField(json, first, "gridCarSelectEnabled", "Grid Car Select",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaEnumField(json, first, "raceViewMode", "Race Mode",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"FULL\"},{\"value\":2,\"label\":\"SIMPLE\"}]");
  appendSchemaEnumField(json, first, "language", "Language",
                        "[{\"value\":0,\"label\":\"NOR\"},{\"value\":1,\"label\":\"ENG\"},{\"value\":2,\"label\":\"CS\"},{\"value\":3,\"label\":\"ACD\"}]");
  appendSchemaEnumField(json, first, "textCase", "Text Case",
                        "[{\"value\":0,\"label\":\"UPPER\"},{\"value\":1,\"label\":\"Pascal\"}]");
  appendSchemaEnumField(json, first, "listFontSize", "Font Size",
                        "[{\"value\":0,\"label\":\"LARGE\"},{\"value\":1,\"label\":\"small\"}]");
  appendSchemaIntField(json, first, "startupDelay", "Startup Delay", STARTUP_DELAY_MIN, STARTUP_DELAY_MAX, 1, "x10ms");
  appendSchemaEnumField(json, first, "statusSlot0", "Status Slot 1",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURRENT\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot1", "Status Slot 2",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURRENT\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot2", "Status Slot 3",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURRENT\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot3", "Status Slot 4",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURRENT\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaStringField(json, first, "screensaverLine1", "Screensaver Line 1", SCREENSAVER_TEXT_MAX - 1);
  appendSchemaStringField(json, first, "screensaverLine2", "Screensaver Line 2", SCREENSAVER_TEXT_MAX - 1);
  json += "],";

  json += "\"car\":[";
  first = true;
  appendSchemaStringField(json, first, "carName", "Profile Name", CAR_NAME_MAX_SIZE - 1);
  appendSchemaIntField(json, first, "minSpeed", "SENSI", 0, MIN_SPEED_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "brake", "BRAKE", 0, BRAKE_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "maxSpeed", "LIMIT", 5, 100, 1, "%");
  appendSchemaIntField(json, first, "curveDiff", "CURVE", THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "antiSpin", "ANTIS", 0, ANTISPIN_MAX_VALUE, 1, "ms");
  appendSchemaIntField(json, first, "freqPWM", "PWM_F", FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100, 1, "x0.1kHz");
  appendSchemaIntField(json, first, "brakeButton", "B_BTN", 0, 100, 1, "%");
  appendSchemaEnumField(json, first, "quickBrakeEnabled", "Q-BRAKE",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaIntField(json, first, "quickBrakeThreshold", "Q-BRAKE Threshold", 0, QUICK_BRAKE_THRESHOLD_MAX, 1, "%");
  appendSchemaIntField(json, first, "quickBrakeStrength", "Q-BRAKE Strength", 0, QUICK_BRAKE_STRENGTH_MAX, 1, "%");
  json += "]";
  json += "}";
  return json;
}

static uint8_t getRequestedCarIndex() {
  if (g_wifiServer != nullptr && g_wifiServer->hasArg("car")) {
    int32_t v = g_wifiServer->arg("car").toInt();
    if (v >= 0 && v < CAR_MAX_COUNT) return (uint8_t)v;
  }
  return (uint8_t)g_storedVar.selectedCarNumber;
}

static String buildStateJson(uint8_t carIndex) {
  if (carIndex >= CAR_MAX_COUNT) carIndex = (uint8_t)g_storedVar.selectedCarNumber;
  const CarParam_type& c = g_storedVar.carParam[carIndex];

  String json;
  json.reserve(2500);
  char buf[160];

  json += "{";
  snprintf(buf, sizeof(buf), "\"selectedCarNumber\":%u,", g_storedVar.selectedCarNumber);
  json += buf;
  snprintf(buf, sizeof(buf), "\"carIndex\":%u,", carIndex);
  json += buf;

  json += "\"global\":{";
  snprintf(buf, sizeof(buf), "\"viewMode\":%u,", g_storedVar.viewMode); json += buf;
  snprintf(buf, sizeof(buf), "\"screensaverTimeout\":%u,", g_storedVar.screensaverTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"powerSaveTimeout\":%u,", g_storedVar.powerSaveTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"deepSleepTimeout\":%u,", g_storedVar.deepSleepTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"soundBoot\":%u,", g_storedVar.soundBoot); json += buf;
  snprintf(buf, sizeof(buf), "\"soundRace\":%u,", g_storedVar.soundRace); json += buf;
  snprintf(buf, sizeof(buf), "\"gridCarSelectEnabled\":%u,", g_storedVar.gridCarSelectEnabled); json += buf;
  snprintf(buf, sizeof(buf), "\"raceViewMode\":%u,", g_storedVar.raceViewMode); json += buf;
  snprintf(buf, sizeof(buf), "\"language\":%u,", g_storedVar.language); json += buf;
  snprintf(buf, sizeof(buf), "\"textCase\":%u,", g_storedVar.textCase); json += buf;
  snprintf(buf, sizeof(buf), "\"listFontSize\":%u,", g_storedVar.listFontSize); json += buf;
  snprintf(buf, sizeof(buf), "\"startupDelay\":%u,", g_storedVar.startupDelay); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot0\":%u,", g_storedVar.statusSlot[0]); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot1\":%u,", g_storedVar.statusSlot[1]); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot2\":%u,", g_storedVar.statusSlot[2]); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot3\":%u,", g_storedVar.statusSlot[3]); json += buf;
  json += "\"screensaverLine1\":\"";
  appendJsonEscaped(json, g_storedVar.screensaverLine1);
  json += "\",\"screensaverLine2\":\"";
  appendJsonEscaped(json, g_storedVar.screensaverLine2);
  json += "\"},";

  json += "\"car\":{";
  json += "\"carName\":\"";
  appendJsonEscaped(json, c.carName);
  json += "\",";
  snprintf(buf, sizeof(buf), "\"minSpeed\":%u,", c.minSpeed); json += buf;
  snprintf(buf, sizeof(buf), "\"brake\":%u,", c.brake); json += buf;
  snprintf(buf, sizeof(buf), "\"maxSpeed\":%u,", c.maxSpeed); json += buf;
  snprintf(buf, sizeof(buf), "\"curveDiff\":%u,", c.throttleCurveVertex.curveSpeedDiff); json += buf;
  snprintf(buf, sizeof(buf), "\"antiSpin\":%u,", c.antiSpin); json += buf;
  snprintf(buf, sizeof(buf), "\"freqPWM\":%u,", c.freqPWM); json += buf;
  snprintf(buf, sizeof(buf), "\"brakeButton\":%u,", c.brakeButtonReduction); json += buf;
  snprintf(buf, sizeof(buf), "\"quickBrakeEnabled\":%u,", c.quickBrakeEnabled); json += buf;
  snprintf(buf, sizeof(buf), "\"quickBrakeThreshold\":%u,", c.quickBrakeThreshold); json += buf;
  snprintf(buf, sizeof(buf), "\"quickBrakeStrength\":%u", c.quickBrakeStrength); json += buf;
  json += "}";
  json += "}";
  return json;
}

static bool parseAndApplyWebPatch(const String& json, String* errorMsg, uint8_t* appliedCarIndex) {
  StoredVar_type updated = g_storedVar;
  int32_t v;
  uint8_t carIndex = updated.selectedCarNumber;

  if (parseJsonInt(json, "carIndex", v)) {
    if (!inRange(v, 0, CAR_MAX_COUNT - 1)) {
      *errorMsg = "Error: invalid carIndex";
      return false;
    }
    carIndex = (uint8_t)v;
  }

  if (parseJsonInt(json, "selectedCarNumber", v)) {
    if (!inRange(v, 0, CAR_MAX_COUNT - 1)) { *errorMsg = "Error: invalid selectedCarNumber"; return false; }
    updated.selectedCarNumber = (uint16_t)v;
  }
  if (parseJsonInt(json, "viewMode", v)) {
    if (!inRange(v, VIEW_MODE_LIST, VIEW_MODE_GRID)) { *errorMsg = "Error: invalid viewMode"; return false; }
    updated.viewMode = (uint16_t)v;
  }
  if (parseJsonInt(json, "screensaverTimeout", v)) {
    if (!inRange(v, 0, SCREENSAVER_TIMEOUT_MAX)) { *errorMsg = "Error: invalid screensaverTimeout"; return false; }
    updated.screensaverTimeout = (uint16_t)v;
  }
  if (parseJsonInt(json, "powerSaveTimeout", v)) {
    if (!inRange(v, 0, POWER_SAVE_TIMEOUT_MAX)) { *errorMsg = "Error: invalid powerSaveTimeout"; return false; }
    updated.powerSaveTimeout = (uint16_t)v;
  }
  if (parseJsonInt(json, "deepSleepTimeout", v)) {
    if (!(v == 0 || inRange(v, DEEP_SLEEP_TIMEOUT_MIN, DEEP_SLEEP_TIMEOUT_MAX))) {
      *errorMsg = "Error: invalid deepSleepTimeout";
      return false;
    }
    updated.deepSleepTimeout = (uint16_t)v;
  }
  if (parseJsonInt(json, "soundBoot", v)) {
    if (!inRange(v, 0, 1)) { *errorMsg = "Error: invalid soundBoot"; return false; }
    updated.soundBoot = (uint16_t)v;
  }
  if (parseJsonInt(json, "soundRace", v)) {
    if (!inRange(v, 0, 1)) { *errorMsg = "Error: invalid soundRace"; return false; }
    updated.soundRace = (uint16_t)v;
  }
  if (parseJsonInt(json, "gridCarSelectEnabled", v)) {
    if (!inRange(v, 0, 1)) { *errorMsg = "Error: invalid gridCarSelectEnabled"; return false; }
    updated.gridCarSelectEnabled = (uint16_t)v;
  }
  if (parseJsonInt(json, "raceViewMode", v)) {
    if (!inRange(v, RACE_VIEW_OFF, RACE_VIEW_SIMPLE)) { *errorMsg = "Error: invalid raceViewMode"; return false; }
    updated.raceViewMode = (uint16_t)v;
  }
  if (parseJsonInt(json, "language", v)) {
    if (!inRange(v, LANG_NOR, LANG_ACD)) { *errorMsg = "Error: invalid language"; return false; }
    updated.language = (uint16_t)v;
  }
  if (parseJsonInt(json, "textCase", v)) {
    if (!inRange(v, TEXT_CASE_UPPER, TEXT_CASE_PASCAL)) { *errorMsg = "Error: invalid textCase"; return false; }
    updated.textCase = (uint16_t)v;
  }
  if (parseJsonInt(json, "listFontSize", v)) {
    if (!inRange(v, FONT_SIZE_LARGE, FONT_SIZE_SMALL)) { *errorMsg = "Error: invalid listFontSize"; return false; }
    updated.listFontSize = (uint16_t)v;
  }
  if (parseJsonInt(json, "startupDelay", v)) {
    if (!inRange(v, STARTUP_DELAY_MIN, STARTUP_DELAY_MAX)) { *errorMsg = "Error: invalid startupDelay"; return false; }
    updated.startupDelay = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot0", v)) {
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot0"; return false; }
    updated.statusSlot[0] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot1", v)) {
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot1"; return false; }
    updated.statusSlot[1] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot2", v)) {
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot2"; return false; }
    updated.statusSlot[2] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot3", v)) {
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot3"; return false; }
    updated.statusSlot[3] = (uint16_t)v;
  }

  char tempStr[SCREENSAVER_TEXT_MAX];
  if (parseJsonStr(json, "screensaverLine1", tempStr, SCREENSAVER_TEXT_MAX)) {
    strncpy(updated.screensaverLine1, tempStr, SCREENSAVER_TEXT_MAX);
    updated.screensaverLine1[SCREENSAVER_TEXT_MAX - 1] = '\0';
  }
  if (parseJsonStr(json, "screensaverLine2", tempStr, SCREENSAVER_TEXT_MAX)) {
    strncpy(updated.screensaverLine2, tempStr, SCREENSAVER_TEXT_MAX);
    updated.screensaverLine2[SCREENSAVER_TEXT_MAX - 1] = '\0';
  }

  CarParam_type car = updated.carParam[carIndex];
  uint16_t minSpeed = car.minSpeed;
  uint16_t maxSpeed = car.maxSpeed;

  if (parseJsonInt(json, "minSpeed", v)) {
    if (!inRange(v, 0, MIN_SPEED_MAX_VALUE)) { *errorMsg = "Error: invalid minSpeed"; return false; }
    minSpeed = (uint16_t)v;
  }
  if (parseJsonInt(json, "maxSpeed", v)) {
    if (!inRange(v, 5, 100)) { *errorMsg = "Error: invalid maxSpeed"; return false; }
    maxSpeed = (uint16_t)v;
  }
  if (maxSpeed < (uint16_t)(minSpeed + 5)) {
    *errorMsg = "Error: maxSpeed must be at least minSpeed+5";
    return false;
  }
  car.minSpeed = minSpeed;
  car.maxSpeed = maxSpeed;

  if (parseJsonInt(json, "brake", v)) {
    if (!inRange(v, 0, BRAKE_MAX_VALUE)) { *errorMsg = "Error: invalid brake"; return false; }
    car.brake = (uint16_t)v;
  }
  if (parseJsonInt(json, "curveDiff", v)) {
    if (!inRange(v, THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE)) {
      *errorMsg = "Error: invalid curveDiff"; return false;
    }
    car.throttleCurveVertex.curveSpeedDiff = (uint16_t)v;
  }
  if (parseJsonInt(json, "antiSpin", v)) {
    if (!inRange(v, 0, ANTISPIN_MAX_VALUE)) { *errorMsg = "Error: invalid antiSpin"; return false; }
    car.antiSpin = (uint16_t)v;
  }
  if (parseJsonInt(json, "freqPWM", v)) {
    if (!inRange(v, FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100)) { *errorMsg = "Error: invalid freqPWM"; return false; }
    car.freqPWM = (uint16_t)v;
  }
  if (parseJsonInt(json, "brakeButton", v)) {
    if (!inRange(v, 0, 100)) { *errorMsg = "Error: invalid brakeButton"; return false; }
    car.brakeButtonReduction = (uint16_t)v;
  }
  if (parseJsonInt(json, "quickBrakeEnabled", v)) {
    if (!inRange(v, 0, 1)) { *errorMsg = "Error: invalid quickBrakeEnabled"; return false; }
    car.quickBrakeEnabled = (uint16_t)v;
  }
  if (parseJsonInt(json, "quickBrakeThreshold", v)) {
    if (!inRange(v, 0, QUICK_BRAKE_THRESHOLD_MAX)) { *errorMsg = "Error: invalid quickBrakeThreshold"; return false; }
    car.quickBrakeThreshold = (uint16_t)v;
  }
  if (parseJsonInt(json, "quickBrakeStrength", v)) {
    if (!inRange(v, 0, QUICK_BRAKE_STRENGTH_MAX)) { *errorMsg = "Error: invalid quickBrakeStrength"; return false; }
    car.quickBrakeStrength = (uint16_t)v;
  }

  char tempName[CAR_NAME_MAX_SIZE];
  if (parseJsonStr(json, "carName", tempName, CAR_NAME_MAX_SIZE)) {
    memset(car.carName, 0, CAR_NAME_MAX_SIZE);
    strncpy(car.carName, tempName, CAR_NAME_MAX_SIZE - 1);
    car.carName[CAR_NAME_MAX_SIZE - 1] = '\0';
  }

  updated.carParam[carIndex] = car;
  g_storedVar = updated;

  if (appliedCarIndex != nullptr) *appliedCarIndex = carIndex;
  return true;
}


/* OTA progress tracking */
static size_t g_otaTotal = 0;
static size_t g_otaWritten = 0;
static bool g_otaInProgress = false;

/* Documentation helpers (served from SPIFFS) */
static const char* getUiPath() {
  return "/ui/index.html";
}

static const char* getDefaultDocsPath() {
  return (g_storedVar.language == LANG_NOR) ? "/docs/no/index.html" : "/docs/en/index.html";
}

static bool streamFileFromSpiffs(const char* path, const char* contentType) {
  if (!g_spiffsMounted) {
    return false;
  }

  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    String gzPath = String(path) + ".gz";
    file = SPIFFS.open(gzPath, FILE_READ);
    if (!file) {
      return false;
    }
    g_wifiServer->sendHeader("Content-Encoding", "gzip");
  }

  g_wifiServer->streamFile(file, contentType);
  file.close();
  return true;
}

static bool streamHtmlFromSpiffs(const char* path) {
  return streamFileFromSpiffs(path, "text/html; charset=utf-8");
}

static int centerX8x8(const char* text) {
  int widthPx = strlen(text) * WIDTH8x8;
  int x = (OLED_WIDTH - widthPx) / 2;
  return (x < 0) ? 0 : x;
}

static void sendUiFallback() {
  g_wifiServer->send(200, "text/html; charset=utf-8", FPSTR(UI_FALLBACK_HTML));
}

static void sendDocsMissing(const char* requestedPath) {
  String page = FPSTR(DOCS_MISSING_HTML);
  page.replace("%PATH%", requestedPath);
  g_wifiServer->send(404, "text/html; charset=utf-8", page);
}

/**
 * @brief Handle a single USB serial backup/restore command.
 * Protocol:
 *   "VERSION"        → "<id>,v<major>.<minor>\n"  e.g. "F0A4,v4.4"
 *   "BACKUP"         → "<bytecount>\n<json>"
 *   "RESTORE\n<len>" → read len bytes, parse, save; reply "OK..." or "ERR:..."
 *   "SCHEMA"         → "<bytecount>\n<json>"
 *   "STATE [car]"    → "<bytecount>\n<json>"
 *   "APPLY\n<len>"   → read len bytes patch JSON; reply "OK\n<bytecount>\n<stateJson>" or "ERR:..."
 *   "SAVE"           → "OK - Saved to flash"
 * Shared by showUSBPortalScreen(); called when Serial.available() triggers.
 */
static void handleSerialCommand(const String& cmd) {
  if (cmd == "VERSION") {
    uint64_t mac = ESP.getEfuseMac();
    char resp[16];
    sprintf(resp, "%02X%02X,v%d.%d", (uint8_t)(mac >> 8), (uint8_t)(mac),
            SW_MAJOR_VERSION, SW_MINOR_VERSION);
    Serial.println(resp);
    Serial.flush();

  } else if (cmd == "BACKUP") {
    String json = buildJsonBackup();
    Serial.print(json.length());
    Serial.print('\n');
    Serial.print(json);
    Serial.flush();

  } else if (cmd == "SCHEMA") {
    String json = buildSchemaJson();
    Serial.print(json.length());
    Serial.print('\n');
    Serial.print(json);
    Serial.flush();

  } else if (cmd.startsWith("STATE")) {
    uint8_t carIndex = (uint8_t)g_storedVar.selectedCarNumber;
    int32_t sp = cmd.indexOf(' ');
    if (sp > 0 && sp < (int32_t)cmd.length() - 1) {
      int32_t v = cmd.substring(sp + 1).toInt();
      if (v < 0 || v >= CAR_MAX_COUNT) {
        Serial.println("ERR:invalid carIndex");
        return;
      }
      carIndex = (uint8_t)v;
    }
    String json = buildStateJson(carIndex);
    Serial.print(json.length());
    Serial.print('\n');
    Serial.print(json);
    Serial.flush();

  } else if (cmd == "APPLY") {
    String lenStr = Serial.readStringUntil('\n');
    int32_t len = lenStr.toInt();
    if (len <= 0 || len > 8192) {
      Serial.println("ERR:invalid length");
      return;
    }
    /* Static buffer avoids heap fragmentation for large payloads */
    static char jsonBuf[8193];
    Serial.setTimeout(15000);
    int32_t got = (int32_t)Serial.readBytes(jsonBuf, (size_t)len);
    Serial.setTimeout(1000);
    if (got != len) { Serial.println("ERR:timeout"); return; }
    jsonBuf[len] = '\0';
    String json(jsonBuf);

    String errorMsg;
    uint8_t carIndex = (uint8_t)g_storedVar.selectedCarNumber;
    if (!parseAndApplyWebPatch(json, &errorMsg, &carIndex)) {
      Serial.println("ERR:" + errorMsg);
      return;
    }

    String outState = buildStateJson(carIndex);
    Serial.println("OK");
    Serial.print(outState.length());
    Serial.print('\n');
    Serial.print(outState);
    Serial.flush();

  } else if (cmd == "SAVE") {
    saveEEPROM(g_storedVar);
    Serial.println("OK - Saved to flash");
    Serial.flush();

  } else if (cmd == "RESTORE") {
    String lenStr = Serial.readStringUntil('\n');
    int32_t len = lenStr.toInt();
    if (len <= 0 || len > 8192) {
      Serial.println("ERR:invalid length");
      return;
    }
    /* Static buffer avoids heap fragmentation for large payloads */
    static char jsonBuf[8193];
    Serial.setTimeout(15000);
    int32_t got = (int32_t)Serial.readBytes(jsonBuf, (size_t)len);
    Serial.setTimeout(1000);
    if (got != len) { Serial.println("ERR:timeout"); return; }
    jsonBuf[len] = '\0';
    String json(jsonBuf);
    StoredVar_type tempVar;
    String errorMsg;
    if (parseAndValidateJson(json, &tempVar, &errorMsg)) {
      g_storedVar = tempVar;
      saveEEPROM(g_storedVar);
      Serial.println("OK - Settings restored");
      Serial.flush();
      delay(500);
      ESP.restart();
    } else {
      Serial.println("ERR:" + errorMsg);
    }
  }
}

/* HTTP route handlers */

static void handleInfo() {
  char ver[8];
  sprintf(ver, "%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);

  String json;
  json.reserve(128);
  json += "{\"deviceId\":\"";
  json += g_wifiSuffix;
  json += "\",\"version\":\"";
  json += ver;
  json += "\",\"docsPath\":\"";
  json += getDefaultDocsPath();
  json += "\"}";

  g_wifiServer->send(200, "application/json", json);
}

static void handleSchema() {
  g_wifiServer->send(200, "application/json", buildSchemaJson());
}

static void handleState() {
  uint8_t carIndex = getRequestedCarIndex();
  g_wifiServer->send(200, "application/json", buildStateJson(carIndex));
}

static void handleApply() {
  String payload = g_wifiServer->arg("plain");
  if (payload.length() == 0) {
    g_wifiServer->send(400, "application/json", "{\"ok\":false,\"error\":\"Missing JSON body\"}");
    return;
  }

  String errorMsg;
  uint8_t carIndex = g_storedVar.selectedCarNumber;
  if (!parseAndApplyWebPatch(payload, &errorMsg, &carIndex)) {
    String out = "{\"ok\":false,\"error\":\"";
    appendJsonEscaped(out, errorMsg.c_str());
    out += "\"}";
    g_wifiServer->send(400, "application/json", out);
    return;
  }

  String out = "{\"ok\":true,\"state\":";
  out += buildStateJson(carIndex);
  out += "}";
  g_wifiServer->send(200, "application/json", out);
}

static void handleSave() {
  saveEEPROM(g_storedVar);
  g_wifiServer->send(200, "application/json", "{\"ok\":true,\"message\":\"Saved to flash\"}");
}

static void handleRoot() {
  if (!streamHtmlFromSpiffs(getUiPath())) {
    sendUiFallback();
  }
}

static void handleUi() {
  if (!streamHtmlFromSpiffs(getUiPath())) {
    sendUiFallback();
  }
}

static void handleBackup() {
  String json = buildJsonBackup();
  g_wifiServer->send(200, "application/json", json);
}

static void handleDocsDefault() {
  const char* defaultPath = getDefaultDocsPath();
  if (!streamHtmlFromSpiffs(defaultPath) &&
      !streamHtmlFromSpiffs("/docs/en/index.html") &&
      !streamHtmlFromSpiffs("/docs/no/index.html") &&
      !streamHtmlFromSpiffs("/docs/es/index.html") &&
      !streamHtmlFromSpiffs("/docs/de/index.html")) {
    sendDocsMissing(defaultPath);
  }
}

static void handleDocsEn() {
  if (!streamHtmlFromSpiffs("/docs/en/index.html")) {
    sendDocsMissing("/docs/en/index.html");
  }
}

static void handleDocsNo() {
  if (!streamHtmlFromSpiffs("/docs/no/index.html")) {
    sendDocsMissing("/docs/no/index.html");
  }
}

static void handleDocsEs() {
  if (!streamHtmlFromSpiffs("/docs/es/index.html")) {
    sendDocsMissing("/docs/es/index.html");
  }
}

static void handleDocsDe() {
  if (!streamHtmlFromSpiffs("/docs/de/index.html")) {
    sendDocsMissing("/docs/de/index.html");
  }
}

static void handleDocsCurveAsset() {
  if (!streamFileFromSpiffs("/docs/assets/curve_examples.png", "image/png")) {
    g_wifiServer->send(404, "text/plain", "Missing asset: /docs/assets/curve_examples.png");
  }
}

static void handleDocsTriggerAsset() {
  if (!streamFileFromSpiffs("/docs/assets/trig_cal.png", "image/png")) {
    g_wifiServer->send(404, "text/plain", "Missing asset: /docs/assets/trig_cal.png");
  }
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
void showWiFiPortalScreen() {
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

  /* Mount SPIFFS for static documentation pages (optional) */
  g_spiffsMounted = SPIFFS.begin(false);

  /* Start web server */
  g_wifiServer = new WebServer(80);
  g_wifiServer->on("/", handleRoot);
  g_wifiServer->on("/index.html", HTTP_GET, handleRoot);
  g_wifiServer->on("/ui", HTTP_GET, handleUi);
  g_wifiServer->on("/ui/", HTTP_GET, handleUi);
  g_wifiServer->on("/ui/index.html", HTTP_GET, handleUi);
  g_wifiServer->on("/api/info", HTTP_GET, handleInfo);
  g_wifiServer->on("/api/schema", HTTP_GET, handleSchema);
  g_wifiServer->on("/api/state", HTTP_GET, handleState);
  g_wifiServer->on("/api/apply", HTTP_POST, handleApply);
  g_wifiServer->on("/api/save", HTTP_POST, handleSave);
  g_wifiServer->on("/backup", HTTP_GET, handleBackup);
  g_wifiServer->on("/docs", HTTP_GET, handleDocsDefault);
  g_wifiServer->on("/docs/", HTTP_GET, handleDocsDefault);
  g_wifiServer->on("/docs/index.html", HTTP_GET, handleDocsDefault);
  g_wifiServer->on("/docs/en", HTTP_GET, handleDocsEn);
  g_wifiServer->on("/docs/en/", HTTP_GET, handleDocsEn);
  g_wifiServer->on("/docs/en/index.html", HTTP_GET, handleDocsEn);
  g_wifiServer->on("/docs/no", HTTP_GET, handleDocsNo);
  g_wifiServer->on("/docs/no/", HTTP_GET, handleDocsNo);
  g_wifiServer->on("/docs/no/index.html", HTTP_GET, handleDocsNo);
  g_wifiServer->on("/docs/es", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/es/", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/es/index.html", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/de", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/de/", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/de/index.html", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/assets/curve_examples.png", HTTP_GET, handleDocsCurveAsset);
  g_wifiServer->on("/docs/assets/trig_cal.png", HTTP_GET, handleDocsTriggerAsset);
  g_wifiServer->on("/restore", HTTP_POST, handleRestore, handleRestoreUpload);
  g_wifiServer->on("/ota", HTTP_POST, handleOta, handleOtaUpload);
  g_wifiServer->begin();

  /* Display WiFi info on OLED (FONT_6x8 = 21 chars/line) */
  obdFill(&g_obd, OBD_WHITE, 1);
  const char* wifiTitle = "WiFi mode";
  obdWriteString(&g_obd, 0, centerX8x8(wifiTitle), 0, (char*)wifiTitle, FONT_8x8, OBD_BLACK, 1);

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
  if (g_spiffsMounted) {
    SPIFFS.end();
    g_spiffsMounted = false;
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * @brief USB backup/restore screen - called from settings menu (USB item).
 * @details Shows a mini guide on the OLED and handles BACKUP/RESTORE serial commands.
 *          No WiFi is started. Returns when user presses encoder button or brake button.
 */
void showUSBPortalScreen() {
  obdFill(&g_obd, OBD_WHITE, 1);
  const char* usbTitle = "USB mode";
  obdWriteString(&g_obd, 0, centerX8x8(usbTitle), 0,  (char*)usbTitle, FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  2 * HEIGHT8x8,  (char*)"1. Connect USB cable",  FONT_6x8,  OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  3 * HEIGHT8x8,  (char*)"2. Open espeed32.html", FONT_6x8,  OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  4 * HEIGHT8x8,  (char*)"   in Chrome/Edge",     FONT_6x8,  OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  5 * HEIGHT8x8,  (char*)"3. Click Connect USB",  FONT_6x8,  OBD_BLACK, 1);

  static bool brakeBtnInUsb = false;
  static uint32_t lastBrakeBtnUsbTime = 0;

  while (true) {
    if (g_rotaryEncoder.isEncoderButtonClicked()) break;

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInUsb && millis() - lastBrakeBtnUsbTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInUsb = true;
        lastBrakeBtnUsbTime = millis();
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    } else {
      brakeBtnInUsb = false;
    }

    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      handleSerialCommand(cmd);
    }

    vTaskDelay(1);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
