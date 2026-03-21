#include "connectivity_portal.h"
#include "slot_ESC.h"
#include "HAL.h"
#include "telemetry_logging.h"
#include <FS.h>
#include <SPIFFS.h>
#include <Update.h>
#include <esp_mac.h>

/* External references from main .ino */
extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern uint16_t g_antiSpinStepMs;
extern uint16_t g_encoderInvertEnabled;
extern uint16_t g_carSel;
extern OBDISP g_obd;
extern AiEsp32RotaryEncoder g_rotaryEncoder;
extern void saveEEPROM(StoredVar_type toSave);

/* Web server instance (heap-allocated only when active) */
static WebServer* g_wifiServer = nullptr;
static String g_uploadBuffer;
static char g_wifiSuffix[5] = "";  /* MAC-based suffix, e.g. "A3B4" */
static bool g_spiffsMounted = false;
static String g_serialCmdLine;

static bool readMacAddress(esp_mac_type_t type, uint8_t out[6]) {
  if (out == nullptr) return false;
  return esp_read_mac(out, type) == ESP_OK;
}

static void appendJsonEscaped(String& out, const char* in);
static String buildTelemetryStatusPayload(const char* message);
static String buildTelemetryLivePayload(uint32_t afterSeq, size_t limit);
static String buildTelemetryConfigSnapshotJson();
static void sendSerialLengthPrefixedPayload(const String& payload);
static const char* getBackupReleaseModeName(uint16_t mode);

static uint16_t normalizeStatusSlotForUi(uint16_t slotValue) {
  return normalizeStatusSlotValue(slotValue);
}

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
<form method="POST" action="/ota-spiffs" enctype="multipart/form-data">
  <p><b>Filesystem Recovery (SPIFFS .bin)</b></p>
  <input type="file" name="file" accept=".bin" required>
  <button type="submit">Upload Filesystem</button>
</form>
<p><b>Restore full web UI:</b></p>
<ol>
  <li>Ensure <code>source/ESPEED32/data/ui/index.html</code> exists.</li>
  <li>Upload SPIFFS image on this page or from IDE.</li>
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
<li>Upload the filesystem image from Web UI or IDE (SPIFFS upload).</li>
<li>Refresh this page.</li>
</ol>
<form method="POST" action="/ota-spiffs" enctype="multipart/form-data">
  <p><b>Upload Filesystem (SPIFFS .bin)</b></p>
  <input type="file" name="file" accept=".bin" required>
  <button type="submit">Upload SPIFFS (.bin)</button>
</form>
<p><a href="/">Back to backup/restore page</a></p>
</body>
</html>
)rawliteral";


/**
 * @brief Build JSON backup string from the provided configuration snapshot.
 */
static String buildJsonBackupFromConfig(const StoredVar_type& storedVar,
                                        uint16_t antiSpinStepMs,
                                        uint16_t encoderInvertEnabled,
                                        uint16_t adcVoltageRange_mV) {
  String json;
  json.reserve(5600);
  char buf[128];

  json += "{\n";
  sprintf(buf, "  \"version\": %d,\n", STORED_VAR_VERSION);              json += buf;
  sprintf(buf, "  \"selectedCarNumber\": %u,\n", storedVar.selectedCarNumber); json += buf;
  sprintf(buf, "  \"minTrigger_raw\": %d,\n", storedVar.minTrigger_raw); json += buf;
  sprintf(buf, "  \"maxTrigger_raw\": %d,\n", storedVar.maxTrigger_raw); json += buf;
  sprintf(buf, "  \"viewMode\": %u,\n", storedVar.viewMode);           json += buf;
  sprintf(buf, "  \"screensaverTimeout\": %u,\n", storedVar.screensaverTimeout); json += buf;
  sprintf(buf, "  \"soundBoot\": %u,\n", storedVar.soundBoot);         json += buf;
  sprintf(buf, "  \"soundRace\": %u,\n", storedVar.soundRace);         json += buf;
  sprintf(buf, "  \"antiSpinStep\": %u,\n", antiSpinStepMs);           json += buf;
  sprintf(buf, "  \"encoderInvert\": %u,\n", encoderInvertEnabled ? 1 : 0); json += buf;
  sprintf(buf, "  \"adcVoltageRangeMv\": %u,\n", adcVoltageRange_mV);  json += buf;
  sprintf(buf, "  \"gridCarSelectEnabled\": %u,\n", storedVar.gridCarSelectEnabled); json += buf;
  sprintf(buf, "  \"raceViewMode\": %u,\n", storedVar.raceViewMode);   json += buf;
  sprintf(buf, "  \"language\": %u,\n", storedVar.language);           json += buf;
  sprintf(buf, "  \"textCase\": %u,\n", storedVar.textCase);           json += buf;
  sprintf(buf, "  \"listFontSize\": %u,\n", storedVar.listFontSize);   json += buf;
  sprintf(buf, "  \"startupDelay\": %u,\n", storedVar.startupDelay);   json += buf;
  json += "  \"screensaverLine1\": \"";
  appendJsonEscaped(json, storedVar.screensaverLine1);
  json += "\",\n";
  json += "  \"screensaverLine2\": \"";
  appendJsonEscaped(json, storedVar.screensaverLine2);
  json += "\",\n";

  json += "  \"cars\": [\n";
  for (int i = 0; i < CAR_MAX_COUNT; i++) {
    const CarParam_type& c = storedVar.carParam[i];
    json += "    {\n";
    json += "      \"name\": \"";
    appendJsonEscaped(json, c.carName);
    json += "\",\n";
    sprintf(buf, "      \"minSpeed\": %u.%u,\n", c.minSpeed / SENSI_SCALE, sensiFracDigit(c.minSpeed)); json += buf;
    sprintf(buf, "      \"brake\": %u,\n", c.brake);                     json += buf;
    sprintf(buf, "      \"maxSpeed\": %u,\n", c.maxSpeed);               json += buf;
    sprintf(buf, "      \"curveInput\": %u,\n", c.throttleCurveVertex.inputThrottle); json += buf;
    sprintf(buf, "      \"curveDiff\": %u,\n", c.throttleCurveVertex.curveSpeedDiff); json += buf;
    sprintf(buf, "      \"fade\": %u,\n", c.fade);                       json += buf;
    sprintf(buf, "      \"antiSpin\": %u,\n", c.antiSpin);               json += buf;
    sprintf(buf, "      \"freqPWM\": %u,\n", c.freqPWM);                 json += buf;
    sprintf(buf, "      \"altBrake\": %u,\n", c.brakeButtonReduction);  json += buf;
    json += "      \"releaseMode\": \"";
    json += getBackupReleaseModeName(c.quickBrakeEnabled);
    json += "\",\n";
    sprintf(buf, "      \"releaseZone\": %u,\n", c.quickBrakeThreshold); json += buf;
    sprintf(buf, "      \"releaseLevel\": %u\n", c.quickBrakeStrength);  json += buf;
    json += "    }";
    if (i < CAR_MAX_COUNT - 1) json += ",";
    json += "\n";
  }
  json += "  ]\n}\n";
  return json;
}

/**
 * @brief Build JSON backup string from current runtime configuration.
 */
static String buildJsonBackup() {
  return buildJsonBackupFromConfig(g_storedVar,
                                   g_antiSpinStepMs,
                                   g_encoderInvertEnabled,
                                   g_adcVoltageRange_mV);
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

static bool parseJsonNumberToken(const String& json, const char* key, String& outToken) {
  String search = String("\"") + key + "\"";
  int idx = json.indexOf(search);
  if (idx < 0) return false;
  int colon = json.indexOf(':', idx);
  if (colon < 0) return false;
  int start = colon + 1;
  while (start < (int)json.length() && (json[start] == ' ' || json[start] == '\n' || json[start] == '\r')) start++;
  int end = start;
  while (end < (int)json.length() && (isDigit(json[end]) || json[end] == '-' || json[end] == '.')) end++;
  if (end == start) return false;
  outToken = json.substring(start, end);
  outToken.trim();
  return outToken.length() > 0;
}

static bool parseJsonHalfPercent(const String& json, const char* key, uint16_t* outRaw) {
  if (outRaw == nullptr) return false;
  String token;
  if (!parseJsonNumberToken(json, key, token)) return false;

  int dot = token.indexOf('.');
  int32_t whole = 0;
  int32_t frac = 0;

  if (dot < 0) {
    whole = token.toInt();
  } else {
    String left = token.substring(0, dot);
    String right = token.substring(dot + 1);
    if (right.length() == 0) return false;
    for (int i = 0; i < (int)right.length(); i++) {
      if (!isDigit(right[i])) return false;
    }
    whole = left.toInt();
    frac = right[0] - '0';
    for (int i = 1; i < (int)right.length(); i++) {
      if (right[i] != '0') return false;  /* allow only one non-zero decimal digit */
    }
  }

  if (whole < 0) return false;
  if (frac != 0 && frac != 5) return false;  /* only 0.0 or 0.5 */

  uint16_t raw = (uint16_t)(whole * SENSI_SCALE + (frac == 5 ? 1 : 0));
  if (raw > MIN_SPEED_MAX_VALUE) return false;
  *outRaw = raw;
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

static bool jsonHasKey(const String& json, const char* key) {
  String search = String("\"") + key + "\"";
  return json.indexOf(search) >= 0;
}

static bool parseJsonIntAlias(const String& json, const char* primaryKey, const char* legacyKey, int32_t& outVal) {
  if (jsonHasKey(json, primaryKey)) return parseJsonInt(json, primaryKey, outVal);
  return parseJsonInt(json, legacyKey, outVal);
}

static const char* getBackupReleaseModeName(uint16_t mode) {
  switch (mode) {
    case RELEASE_BRAKE_QUICK: return "QUICK";
    case RELEASE_BRAKE_DRAG:  return "DRAG";
    case RELEASE_BRAKE_OFF:
    default:                  return "OFF";
  }
}

static bool parseJsonReleaseMode(const String& json, const char* key, uint16_t* outMode) {
  if (outMode == nullptr) return false;

  int32_t v = 0;
  if (parseJsonInt(json, key, v)) {
    if (v < RELEASE_BRAKE_OFF || v > RELEASE_BRAKE_DRAG) return false;
    *outMode = (uint16_t)v;
    return true;
  }

  char modeStr[16];
  if (!parseJsonStr(json, key, modeStr, sizeof(modeStr))) return false;

  String mode = modeStr;
  mode.trim();
  if (mode.equalsIgnoreCase("OFF")) {
    *outMode = RELEASE_BRAKE_OFF;
    return true;
  }
  if (mode.equalsIgnoreCase("QUICK") || mode.equalsIgnoreCase("QCK")) {
    *outMode = RELEASE_BRAKE_QUICK;
    return true;
  }
  if (mode.equalsIgnoreCase("DRAG") || mode.equalsIgnoreCase("DRG")) {
    *outMode = RELEASE_BRAKE_DRAG;
    return true;
  }

  return false;
}

static bool parseJsonReleaseModeAlias(const String& json, const char* primaryKey, const char* legacyKey, uint16_t* outMode) {
  if (jsonHasKey(json, primaryKey)) return parseJsonReleaseMode(json, primaryKey, outMode);
  return parseJsonReleaseMode(json, legacyKey, outMode);
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
static bool parseAndValidateJson(const String& json, StoredVar_type* sv, uint16_t* antiSpinStepMs,
                                 uint16_t* encoderInvertEnabled, uint16_t* adcVoltageRangeMv, String* errorMsg) {
  int32_t v;

  /* Initialize with current settings as defaults (missing fields keep current values) */
  *sv = g_storedVar;
  if (antiSpinStepMs != nullptr) {
    *antiSpinStepMs = g_antiSpinStepMs;
  }
  if (encoderInvertEnabled != nullptr) {
    *encoderInvertEnabled = g_encoderInvertEnabled ? 1 : 0;
  }
  if (adcVoltageRangeMv != nullptr) {
    *adcVoltageRangeMv = g_adcVoltageRange_mV;
  }

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
  if (antiSpinStepMs != nullptr &&
      parseJsonInt(json, "antiSpinStep", v) &&
      inRange(v, ANTISPIN_STEP_MIN, ANTISPIN_STEP_MAX)) {
    *antiSpinStepMs = (uint16_t)v;
  }
  if (encoderInvertEnabled != nullptr &&
      parseJsonInt(json, "encoderInvert", v) &&
      inRange(v, 0, 1)) {
    *encoderInvertEnabled = (uint16_t)v;
  }
  if (adcVoltageRangeMv != nullptr &&
      parseJsonInt(json, "adcVoltageRangeMv", v) &&
      inRange(v, ADC_VOLTAGE_RANGE_MIN_MVOLTS, ADC_VOLTAGE_RANGE_MAX_MVOLTS)) {
    *adcVoltageRangeMv = (uint16_t)v;
  }
  if (parseJsonInt(json, "gridCarSelectEnabled", v) && inRange(v, 0, 1))
    sv->gridCarSelectEnabled = v;
  if (parseJsonInt(json, "raceViewMode", v) && inRange(v, 0, 2))
    sv->raceViewMode = v;
  if (parseJsonInt(json, "language", v) && inRange(v, LANG_NOR, LANG_MAX))
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
    uint16_t minSpeedRaw = 0;
    if (!parseJsonHalfPercent(carJson, "minSpeed", &minSpeedRaw)) {
      *errorMsg = "Error: invalid minSpeed in car " + String(i); return false;
    }
    c.minSpeed = minSpeedRaw;

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

    if (parseJsonInt(carJson, "fade", v) && inRange(v, 0, FADE_MAX_VALUE))
      c.fade = v;

    if (!parseJsonInt(carJson, "antiSpin", v) || !inRange(v, 0, ANTISPIN_MAX_VALUE)) {
      *errorMsg = "Error: invalid antiSpin in car " + String(i); return false;
    }
    c.antiSpin = v;

    if (!parseJsonInt(carJson, "freqPWM", v) || !inRange(v, FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100)) {
      *errorMsg = "Error: invalid freqPWM in car " + String(i); return false;
    }
    c.freqPWM = v;

    if (!parseJsonIntAlias(carJson, "altBrake", "brakeButton", v) || !inRange(v, 0, 100)) {
      *errorMsg = "Error: invalid altBrake in car " + String(i); return false;
    }
    c.brakeButtonReduction = v;

    /* Release-brake fields — optional for backwards compatibility with older backups */
    uint16_t releaseMode = c.quickBrakeEnabled;
    if (parseJsonReleaseModeAlias(carJson, "releaseMode", "quickBrakeEnabled", &releaseMode)) {
      c.quickBrakeEnabled = releaseMode;
    } else if (jsonHasKey(carJson, "releaseMode") || jsonHasKey(carJson, "quickBrakeEnabled")) {
      *errorMsg = "Error: invalid releaseMode in car " + String(i); return false;
    }

    if (parseJsonIntAlias(carJson, "releaseZone", "quickBrakeThreshold", v)) {
      if (!inRange(v, 0, QUICK_BRAKE_THRESHOLD_MAX)) {
        *errorMsg = "Error: invalid releaseZone in car " + String(i); return false;
      }
      c.quickBrakeThreshold = v;
    } else if (jsonHasKey(carJson, "releaseZone") || jsonHasKey(carJson, "quickBrakeThreshold")) {
      *errorMsg = "Error: invalid releaseZone in car " + String(i); return false;
    }

    if (parseJsonIntAlias(carJson, "releaseLevel", "quickBrakeStrength", v)) {
      if (!inRange(v, 0, QUICK_BRAKE_STRENGTH_MAX)) {
        *errorMsg = "Error: invalid releaseLevel in car " + String(i); return false;
      }
      c.quickBrakeStrength = v;
    } else if (jsonHasKey(carJson, "releaseLevel") || jsonHasKey(carJson, "quickBrakeStrength")) {
      *errorMsg = "Error: invalid releaseLevel in car " + String(i); return false;
    }
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

static void appendHalfPercentField(String& out, const char* key, uint16_t sensiRaw, bool withTrailingComma) {
  char buf[48];
  snprintf(buf, sizeof(buf), "\"%s\":%u.%u%s",
           key,
           sensiRaw / SENSI_SCALE,
           sensiFracDigit(sensiRaw),
           withTrailingComma ? "," : "");
  out += buf;
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

static void appendSchemaNumberField(
  String& out, bool& first, const char* id, const char* label,
  const char* minVal, const char* maxVal, const char* step, const char* unit = nullptr) {
  if (!first) out += ",";
  first = false;
  out += "{\"id\":\"";
  out += id;
  out += "\",\"label\":\"";
  out += label;
  out += "\",\"type\":\"int\",\"min\":";
  out += minVal;
  out += ",\"max\":";
  out += maxVal;
  out += ",\"step\":";
  out += step;
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
  json.reserve(5500);

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
  appendSchemaIntField(json, first, "antiSpinStep", "ANTIS Step", ANTISPIN_STEP_MIN, ANTISPIN_STEP_MAX, 1, "ms");
  appendSchemaEnumField(json, first, "encoderInvert", "ENC INV",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaIntField(json, first, "adcVoltageRangeMv", "VIN CAL ADC", ADC_VOLTAGE_RANGE_MIN_MVOLTS, ADC_VOLTAGE_RANGE_MAX_MVOLTS, 1, "mV");
  appendSchemaEnumField(json, first, "gridCarSelectEnabled", "Grid Car Select",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"ON\"}]");
  appendSchemaEnumField(json, first, "raceViewMode", "Race Mode",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"FULL\"},{\"value\":2,\"label\":\"SIMPLE\"}]");
  appendSchemaEnumField(json, first, "language", "Language",
                        "[{\"value\":0,\"label\":\"NOR\"},{\"value\":1,\"label\":\"ENG\"},{\"value\":2,\"label\":\"CS\"},{\"value\":3,\"label\":\"ACD\"},{\"value\":4,\"label\":\"ESP\"},{\"value\":5,\"label\":\"DEU\"},{\"value\":6,\"label\":\"ITA\"},{\"value\":7,\"label\":\"NLD\"}]");
  appendSchemaEnumField(json, first, "textCase", "Text Case",
                        "[{\"value\":0,\"label\":\"UPPER\"},{\"value\":1,\"label\":\"Pascal\"}]");
  appendSchemaEnumField(json, first, "listFontSize", "Font Size",
                        "[{\"value\":0,\"label\":\"LARGE\"},{\"value\":1,\"label\":\"small\"}]");
  appendSchemaIntField(json, first, "startupDelay", "Startup Delay", STARTUP_DELAY_MIN, STARTUP_DELAY_MAX, 1, "x10ms");
  appendSchemaEnumField(json, first, "statusSlot0", "Status Slot 1",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURR\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot1", "Status Slot 2",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURR\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot2", "Status Slot 3",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURR\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaEnumField(json, first, "statusSlot3", "Status Slot 4",
                        "[{\"value\":0,\"label\":\"BLANK\"},{\"value\":1,\"label\":\"OUTPUT\"},{\"value\":2,\"label\":\"THROTTLE\"},{\"value\":3,\"label\":\"CAR\"},{\"value\":4,\"label\":\"CURR\"},{\"value\":5,\"label\":\"VOLTAGE\"}]");
  appendSchemaStringField(json, first, "screensaverLine1", "Screensaver Line 1", SCREENSAVER_TEXT_MAX - 1);
  appendSchemaStringField(json, first, "screensaverLine2", "Screensaver Line 2", SCREENSAVER_TEXT_MAX - 1);
  json += "],";

  json += "\"car\":[";
  first = true;
  appendSchemaStringField(json, first, "carName", "Profile Name", CAR_NAME_MAX_SIZE - 1);
  appendSchemaNumberField(json, first, "minSpeed", "SENSI", "0.0", "90.0", "0.5", "%");
  appendSchemaIntField(json, first, "brake", "BRAKE", 0, BRAKE_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "maxSpeed", "LIMIT", 5, 100, 1, "%");
  appendSchemaIntField(json, first, "curveDiff", "CURVE", THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "fade", "FADE", 0, FADE_MAX_VALUE, 1, "%");
  appendSchemaIntField(json, first, "antiSpin", "ANTIS", 0, ANTISPIN_MAX_VALUE, 1, "ms");
  appendSchemaIntField(json, first, "freqPWM", "PWM_F", FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100, 1, "x0.1kHz");
  appendSchemaIntField(json, first, "brakeButton", "Alt.Brake", 0, 100, 1, "%");
  appendSchemaEnumField(json, first, "quickBrakeEnabled", "Rel.Brake",
                        "[{\"value\":0,\"label\":\"OFF\"},{\"value\":1,\"label\":\"QUICK\"},{\"value\":2,\"label\":\"DRAG\"}]");
  appendSchemaIntField(json, first, "quickBrakeThreshold", "Rel.Brake Zone", 0, QUICK_BRAKE_THRESHOLD_MAX, 1, "%");
  appendSchemaIntField(json, first, "quickBrakeStrength", "Rel.Brake Level", 0, QUICK_BRAKE_STRENGTH_MAX, 1, "%");
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
  json.reserve(2600);
  char buf[160];

  json += "{";
  snprintf(buf, sizeof(buf), "\"selectedCarNumber\":%u,", g_storedVar.selectedCarNumber);
  json += buf;
  snprintf(buf, sizeof(buf), "\"carIndex\":%u,", carIndex);
  json += buf;

  json += "\"global\":{";
  snprintf(buf, sizeof(buf), "\"selectedCarNumber\":%u,", g_storedVar.selectedCarNumber); json += buf;
  snprintf(buf, sizeof(buf), "\"viewMode\":%u,", g_storedVar.viewMode); json += buf;
  snprintf(buf, sizeof(buf), "\"screensaverTimeout\":%u,", g_storedVar.screensaverTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"powerSaveTimeout\":%u,", g_storedVar.powerSaveTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"deepSleepTimeout\":%u,", g_storedVar.deepSleepTimeout); json += buf;
  snprintf(buf, sizeof(buf), "\"soundBoot\":%u,", g_storedVar.soundBoot); json += buf;
  snprintf(buf, sizeof(buf), "\"soundRace\":%u,", g_storedVar.soundRace); json += buf;
  snprintf(buf, sizeof(buf), "\"antiSpinStep\":%u,", g_antiSpinStepMs); json += buf;
  snprintf(buf, sizeof(buf), "\"encoderInvert\":%u,", g_encoderInvertEnabled ? 1 : 0); json += buf;
  snprintf(buf, sizeof(buf), "\"adcVoltageRangeMv\":%u,", g_adcVoltageRange_mV); json += buf;
  snprintf(buf, sizeof(buf), "\"gridCarSelectEnabled\":%u,", g_storedVar.gridCarSelectEnabled); json += buf;
  snprintf(buf, sizeof(buf), "\"raceViewMode\":%u,", g_storedVar.raceViewMode); json += buf;
  snprintf(buf, sizeof(buf), "\"language\":%u,", g_storedVar.language); json += buf;
  snprintf(buf, sizeof(buf), "\"textCase\":%u,", g_storedVar.textCase); json += buf;
  snprintf(buf, sizeof(buf), "\"listFontSize\":%u,", g_storedVar.listFontSize); json += buf;
  snprintf(buf, sizeof(buf), "\"startupDelay\":%u,", g_storedVar.startupDelay); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot0\":%u,", normalizeStatusSlotForUi(g_storedVar.statusSlot[0])); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot1\":%u,", normalizeStatusSlotForUi(g_storedVar.statusSlot[1])); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot2\":%u,", normalizeStatusSlotForUi(g_storedVar.statusSlot[2])); json += buf;
  snprintf(buf, sizeof(buf), "\"statusSlot3\":%u,", normalizeStatusSlotForUi(g_storedVar.statusSlot[3])); json += buf;
  json += "\"screensaverLine1\":\"";
  appendJsonEscaped(json, g_storedVar.screensaverLine1);
  json += "\",\"screensaverLine2\":\"";
  appendJsonEscaped(json, g_storedVar.screensaverLine2);
  json += "\"},";

  json += "\"car\":{";
  json += "\"carName\":\"";
  appendJsonEscaped(json, c.carName);
  json += "\",";
  appendHalfPercentField(json, "minSpeed", c.minSpeed, true);
  snprintf(buf, sizeof(buf), "\"brake\":%u,", c.brake); json += buf;
  snprintf(buf, sizeof(buf), "\"maxSpeed\":%u,", c.maxSpeed); json += buf;
  snprintf(buf, sizeof(buf), "\"curveDiff\":%u,", c.throttleCurveVertex.curveSpeedDiff); json += buf;
  snprintf(buf, sizeof(buf), "\"fade\":%u,", c.fade); json += buf;
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
  if (parseJsonInt(json, "antiSpinStep", v)) {
    if (!inRange(v, ANTISPIN_STEP_MIN, ANTISPIN_STEP_MAX)) { *errorMsg = "Error: invalid antiSpinStep"; return false; }
    g_antiSpinStepMs = (uint16_t)v;
  }
  if (parseJsonInt(json, "encoderInvert", v)) {
    if (!inRange(v, 0, 1)) { *errorMsg = "Error: invalid encoderInvert"; return false; }
    applyEncoderInvertSetting((uint16_t)v);
  }
  if (parseJsonInt(json, "adcVoltageRangeMv", v)) {
    if (!inRange(v, ADC_VOLTAGE_RANGE_MIN_MVOLTS, ADC_VOLTAGE_RANGE_MAX_MVOLTS)) {
      *errorMsg = "Error: invalid adcVoltageRangeMv";
      return false;
    }
    applyAdcVoltageRangeMilliVolts((uint16_t)v);
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
    if (!inRange(v, LANG_NOR, LANG_MAX)) { *errorMsg = "Error: invalid language"; return false; }
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
    if (v == STATUS_CURRENT_MA) v = STATUS_CURRENT;
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot0"; return false; }
    updated.statusSlot[0] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot1", v)) {
    if (v == STATUS_CURRENT_MA) v = STATUS_CURRENT;
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot1"; return false; }
    updated.statusSlot[1] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot2", v)) {
    if (v == STATUS_CURRENT_MA) v = STATUS_CURRENT;
    if (!inRange(v, STATUS_BLANK, STATUS_VOLTAGE)) { *errorMsg = "Error: invalid statusSlot2"; return false; }
    updated.statusSlot[2] = (uint16_t)v;
  }
  if (parseJsonInt(json, "statusSlot3", v)) {
    if (v == STATUS_CURRENT_MA) v = STATUS_CURRENT;
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
  uint16_t minSpeedRaw = car.minSpeed;
  uint16_t maxSpeed = car.maxSpeed;

  if (json.indexOf("\"minSpeed\"") >= 0) {
    if (!parseJsonHalfPercent(json, "minSpeed", &minSpeedRaw)) { *errorMsg = "Error: invalid minSpeed"; return false; }
  }
  if (parseJsonInt(json, "maxSpeed", v)) {
    if (!inRange(v, 5, 100)) { *errorMsg = "Error: invalid maxSpeed"; return false; }
    maxSpeed = (uint16_t)v;
  }
  if (maxSpeed < (uint16_t)(sensiToWholePctCeil(minSpeedRaw) + 5)) {
    *errorMsg = "Error: maxSpeed must be at least minSpeed+5";
    return false;
  }
  car.minSpeed = minSpeedRaw;
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
  if (parseJsonInt(json, "fade", v)) {
    if (!inRange(v, 0, FADE_MAX_VALUE)) { *errorMsg = "Error: invalid fade"; return false; }
    car.fade = (uint16_t)v;
  }
  if (parseJsonInt(json, "antiSpin", v)) {
    if (!inRange(v, 0, ANTISPIN_MAX_VALUE)) { *errorMsg = "Error: invalid antiSpin"; return false; }
    car.antiSpin = (uint16_t)v;
  }
  if (parseJsonInt(json, "freqPWM", v)) {
    if (!inRange(v, FREQ_MIN_VALUE / 100, FREQ_MAX_VALUE / 100)) { *errorMsg = "Error: invalid freqPWM"; return false; }
    car.freqPWM = (uint16_t)v;
  }
  if (jsonHasKey(json, "altBrake") || jsonHasKey(json, "brakeButton")) {
    if (!parseJsonIntAlias(json, "altBrake", "brakeButton", v) || !inRange(v, 0, 100)) {
      *errorMsg = "Error: invalid altBrake"; return false;
    }
    car.brakeButtonReduction = (uint16_t)v;
  }
  if (jsonHasKey(json, "releaseMode") || jsonHasKey(json, "quickBrakeEnabled")) {
    uint16_t releaseMode = car.quickBrakeEnabled;
    if (!parseJsonReleaseModeAlias(json, "releaseMode", "quickBrakeEnabled", &releaseMode)) {
      *errorMsg = "Error: invalid releaseMode"; return false;
    }
    car.quickBrakeEnabled = releaseMode;
  }
  if (jsonHasKey(json, "releaseZone") || jsonHasKey(json, "quickBrakeThreshold")) {
    if (!parseJsonIntAlias(json, "releaseZone", "quickBrakeThreshold", v) || !inRange(v, 0, QUICK_BRAKE_THRESHOLD_MAX)) {
      *errorMsg = "Error: invalid releaseZone"; return false;
    }
    car.quickBrakeThreshold = (uint16_t)v;
  }
  if (jsonHasKey(json, "releaseLevel") || jsonHasKey(json, "quickBrakeStrength")) {
    if (!parseJsonIntAlias(json, "releaseLevel", "quickBrakeStrength", v) || !inRange(v, 0, QUICK_BRAKE_STRENGTH_MAX)) {
      *errorMsg = "Error: invalid releaseLevel"; return false;
    }
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
static bool g_otaTargetSpiffs = false;
static bool g_otaSessionOk = false;

/* Documentation helpers (served from SPIFFS) */
static const char* getUiPath() {
  return "/ui/index.html";
}

static const char* getDocsPathForLanguage(uint16_t lang) {
  switch (lang) {
    case LANG_NOR: return "/docs/no/index.html";
    case LANG_ESP: return "/docs/es/index.html";
    case LANG_DEU: return "/docs/de/index.html";
    case LANG_ITA: return "/docs/it/index.html";
    case LANG_NLD: return "/docs/nl/index.html";
    default:       return "/docs/en/index.html";
  }
}

static const char* getDefaultDocsPath() {
  return getDocsPathForLanguage(g_storedVar.language);
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

int centerX8x8(const char* text) {
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

static void handleSerialCommand(const String& cmd);

/**
 * @brief Non-blocking USB serial command pump.
 * @details Reads one newline-terminated command and dispatches it. Called frequently
 *          from normal UI loops so WebSerial works outside the USB info screen too.
 */
static void serviceUsbSerialCommands() {
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();

    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      if (g_serialCmdLine.length() > 0) {
        String cmd = g_serialCmdLine;
        g_serialCmdLine = "";
        cmd.trim();
        if (cmd.length() > 0) {
          handleSerialCommand(cmd);
          return;  /* handle one command per call */
        }
      }
      continue;
    }

    if (g_serialCmdLine.length() < 64) {
      g_serialCmdLine += ch;
    } else {
      /* Corrupted/too-long command line: drop until next newline. */
      g_serialCmdLine = "";
    }
  }
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
 *   "TSTATUS"        → "<bytecount>\n<json>"
 *   "TLIVE a l"      → "<bytecount>\n<json>" (afterSeq=a, limit=l)
 *   "TSTART"         → "<bytecount>\n<json>"
 *   "TSTOP"          → "<bytecount>\n<json>"
 *   "TCLEAR"         → "<bytecount>\n<json>"
 *   "TCONFIG"        → "<bytecount>\n<json>"
 * Shared by showUSBPortalScreen(); called when Serial.available() triggers.
 */
static void handleSerialCommand(const String& cmd) {
  if (cmd == "VERSION") {
    uint8_t idHi = 0;
    uint8_t idLo = 0;
    uint8_t wifiMac[6] = {0};
    if (readMacAddress(ESP_MAC_WIFI_STA, wifiMac)) {
      idHi = wifiMac[4];
      idLo = wifiMac[5];
    } else {
      uint64_t mac = ESP.getEfuseMac();
      idHi = (uint8_t)(mac >> 8);
      idLo = (uint8_t)(mac);
    }
    char resp[16];
    sprintf(resp, "%02X%02X,v%d.%d", idHi, idLo,
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
    sendSerialLengthPrefixedPayload(json);

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
    sendSerialLengthPrefixedPayload(json);

  } else if (cmd == "TSTATUS") {
    sendSerialLengthPrefixedPayload(buildTelemetryStatusPayload(""));

  } else if (cmd.startsWith("TLIVE")) {
    uint32_t afterSeq = 0U;
    size_t limit = 256U;
    int32_t firstSpace = cmd.indexOf(' ');
    if (firstSpace > 0 && firstSpace < (int32_t)cmd.length() - 1) {
      int32_t secondSpace = cmd.indexOf(' ', firstSpace + 1);
      String afterStr;
      String limitStr;
      if (secondSpace > firstSpace) {
        afterStr = cmd.substring(firstSpace + 1, secondSpace);
        limitStr = cmd.substring(secondSpace + 1);
      } else {
        afterStr = cmd.substring(firstSpace + 1);
      }
      afterStr.trim();
      limitStr.trim();
      if (afterStr.length() > 0) {
        afterSeq = (uint32_t)afterStr.toInt();
      }
      if (limitStr.length() > 0) {
        int32_t parsedLimit = limitStr.toInt();
        if (parsedLimit > 0) {
          limit = (size_t)parsedLimit;
        }
      }
    }
    if (limit < 1U) limit = 1U;
    if (limit > 256U) limit = 256U;
    sendSerialLengthPrefixedPayload(buildTelemetryLivePayload(afterSeq, limit));

  } else if (cmd == "TSTART") {
    if (!telemetryStartLogging(&g_storedVar,
                               g_antiSpinStepMs,
                               g_encoderInvertEnabled,
                               g_adcVoltageRange_mV,
                               (uint8_t)g_carSel)) {
      Serial.println("ERR:Telemetry start failed");
      return;
    }
    sendSerialLengthPrefixedPayload(buildTelemetryStatusPayload("Telemetry logging started"));

  } else if (cmd == "TSTOP") {
    telemetryStopLogging();
    sendSerialLengthPrefixedPayload(buildTelemetryStatusPayload("Telemetry logging stopped"));

  } else if (cmd == "TCLEAR") {
    telemetryStopLogging();
    telemetryClear();
    sendSerialLengthPrefixedPayload(buildTelemetryStatusPayload("Telemetry buffer cleared"));

  } else if (cmd == "TCONFIG") {
    sendSerialLengthPrefixedPayload(buildTelemetryConfigSnapshotJson());

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
    uint16_t tempAntiSpinStep = g_antiSpinStepMs;
    uint16_t tempEncoderInvert = g_encoderInvertEnabled ? 1 : 0;
    uint16_t tempAdcVoltageRange = g_adcVoltageRange_mV;
    if (parseAndValidateJson(json, &tempVar, &tempAntiSpinStep, &tempEncoderInvert, &tempAdcVoltageRange, &errorMsg)) {
      g_storedVar = tempVar;
      g_antiSpinStepMs = tempAntiSpinStep;
      g_encoderInvertEnabled = tempEncoderInvert ? 1 : 0;
      applyAdcVoltageRangeMilliVolts(tempAdcVoltageRange);
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

static uint32_t getTelemetryArgU32(const char* name, uint32_t defaultValue) {
  if (g_wifiServer == nullptr || name == nullptr || !g_wifiServer->hasArg(name)) {
    return defaultValue;
  }

  String raw = g_wifiServer->arg(name);
  raw.trim();
  if (raw.length() == 0) {
    return defaultValue;
  }

  bool valid = true;
  for (uint16_t i = 0; i < raw.length(); i++) {
    if (!isDigit(raw[i])) {
      valid = false;
      break;
    }
  }
  if (!valid) {
    return defaultValue;
  }

  return (uint32_t)raw.toInt();
}

static size_t getTelemetryLiveLimit() {
  uint32_t limit = getTelemetryArgU32("limit", 120U);
  if (limit == 0U) {
    limit = 120U;
  }
  if (limit > 256U) {
    limit = 256U;
  }
  return (size_t)limit;
}

static void formatHalfPercentValue(uint16_t raw, char* out, size_t outLen) {
  if (out == nullptr || outLen == 0) {
    return;
  }
  snprintf(out, outLen, "%u.%u", raw / SENSI_SCALE, sensiFracDigit(raw));
}

static const char* getTelemetryReleaseModeLabel(uint8_t mode) {
  switch (mode) {
    case RELEASE_BRAKE_QUICK: return "QUICK";
    case RELEASE_BRAKE_DRAG:  return "DRAG";
    default:                  return "OFF";
  }
}

static void appendTelemetryCarParamJson(String& json, const CarParam_type& car) {
  char buf[256];
  json += "{";
  json += "\"name\":\"";
  appendJsonEscaped(json, car.carName);
  json += "\"";
  snprintf(buf, sizeof(buf),
           ",\"carNumber\":%u,\"minSpeedHalfPct\":%u,\"brake\":%u,\"maxSpeed\":%u,"
           "\"curveInputPct\":%u,\"curveDiffPct\":%u,\"fade\":%u,\"antiSpin\":%u,"
           "\"freqPwm100Hz\":%u,\"brakeButton\":%u,\"releaseMode\":%u,\"releaseZone\":%u,\"releaseLevel\":%u",
           (unsigned int)car.carNumber,
           (unsigned int)car.minSpeed,
           (unsigned int)car.brake,
           (unsigned int)car.maxSpeed,
           (unsigned int)car.throttleCurveVertex.inputThrottle,
           (unsigned int)car.throttleCurveVertex.curveSpeedDiff,
           (unsigned int)car.fade,
           (unsigned int)car.antiSpin,
           (unsigned int)car.freqPWM,
           (unsigned int)car.brakeButtonReduction,
           (unsigned int)car.quickBrakeEnabled,
           (unsigned int)car.quickBrakeThreshold,
           (unsigned int)car.quickBrakeStrength);
  json += buf;
  json += "}";
}

static void appendTelemetryCarNamesJson(String& json, const StoredVar_type& storedVar) {
  json += "[";
  for (uint8_t i = 0; i < CAR_MAX_COUNT; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "\"";
    appendJsonEscaped(json, storedVar.carParam[i].carName);
    json += "\"";
  }
  json += "]";
}

static String buildTelemetryConfigSummaryJsonFromSnapshot(const TelemetryConfigSnapshot& snapshot,
                                                          uint8_t activeCarIndex) {
  if (activeCarIndex >= CAR_MAX_COUNT) {
    activeCarIndex = 0;
  }

  String json;
  json.reserve(960);
  json += "{";
  json += "\"selectedCarNumber\":";
  json += String(activeCarIndex);
  json += ",\"antiSpinStepMs\":";
  json += String(snapshot.antiSpinStepMs);
  json += ",\"encoderInvert\":";
  json += String(snapshot.encoderInvertEnabled ? 1U : 0U);
  json += ",\"adcVoltageRangeMv\":";
  json += String(snapshot.adcVoltageRange_mV);
  json += ",\"statusSlots\":[";
  for (uint8_t i = 0; i < STATUS_SLOTS; i++) {
    if (i > 0) {
      json += ",";
    }
    json += String(normalizeStatusSlotForUi(snapshot.storedVar.statusSlot[i]));
  }
  json += "],\"carNames\":";
  appendTelemetryCarNamesJson(json, snapshot.storedVar);
  json += ",\"activeCar\":";
  appendTelemetryCarParamJson(json, snapshot.storedVar.carParam[activeCarIndex]);
  json += "}";
  return json;
}

static void appendTelemetryEventJson(String& json, const TelemetryEvent& event) {
  json += "{";
  json += "\"id\":";
  json += String((unsigned long)event.id);
  json += ",\"tMs\":";
  json += String((unsigned long)event.t_ms);
  json += ",\"sampleSeq\":";
  json += String((unsigned long)event.sampleSeq);
  json += ",\"type\":\"";
  json += (event.type == TELEMETRY_EVENT_CAR_SELECT) ? "car_select" : "car_params";
  json += "\",\"carIndex\":";
  json += String((unsigned int)event.carIndex);
  json += ",\"previousCarIndex\":";
  json += String((unsigned int)event.previousCarIndex);
  json += ",\"changedMask\":";
  json += String((unsigned int)event.changedMask);
  json += ",\"carParams\":";
  appendTelemetryCarParamJson(json, event.carParam);
  json += "}";
}

static void appendTelemetryStatusFields(String& json, const TelemetryStatus& status) {
  char buf[320];
  uint8_t sessionCarIndex = (status.sessionStartCarIndex < CAR_MAX_COUNT) ? status.sessionStartCarIndex : 0;
  uint8_t currentCarIndex = (g_carSel < CAR_MAX_COUNT) ? (uint8_t)g_carSel : (uint8_t)g_storedVar.selectedCarNumber;
  uint8_t triggerPct = (uint8_t)(((uint32_t)g_escVar.trigger_norm * 100U) / THROTTLE_NORMALIZED);
  uint8_t outputPct = (uint8_t)constrain((int)g_escVar.outputSpeed_pct, 0, 100);
  uint8_t brakePct = (uint8_t)constrain((int)g_escVar.effectiveBrake_pct, 0, 100);
  if (digitalRead(BUTT_PIN) == BUTTON_PRESSED && g_escVar.trigger_norm == 0) {
    brakePct = (uint8_t)constrain((int)g_storedVar.carParam[currentCarIndex].brakeButtonReduction, 0, 100);
  }

  snprintf(buf, sizeof(buf),
           ",\"loggingActive\":%u,\"hasData\":%u,\"wrapped\":%u,\"sampleRateMs\":%u,\"capacity\":%u,"
           "\"storedCount\":%u,\"eventCount\":%u,\"eventCapacity\":%u,"
           "\"sessionId\":%lu,\"sessionStartMs\":%lu,\"oldestSeq\":%lu,\"latestSeq\":%lu,"
           "\"oldestEventId\":%lu,\"latestEventId\":%lu,"
           "\"wifiActive\":%u,\"sessionStartCarIndex\":%u,\"currentCarIndex\":%u",
           status.loggingActive ? 1U : 0U,
           status.hasData ? 1U : 0U,
           status.wrapped ? 1U : 0U,
           status.sampleRateMs,
           status.capacity,
           status.storedCount,
           status.eventCount,
           status.eventCapacity,
           (unsigned long)status.sessionId,
           (unsigned long)status.sessionStartMs,
           (unsigned long)status.oldestSeq,
           (unsigned long)status.latestSeq,
           (unsigned long)status.oldestEventId,
           (unsigned long)status.latestEventId,
           isWiFiPortalActive() ? 1U : 0U,
           sessionCarIndex,
           currentCarIndex);
  json += buf;

  json += ",\"sessionStartCarName\":\"";
  appendJsonEscaped(json, g_storedVar.carParam[sessionCarIndex].carName);
  json += "\",\"currentCarName\":\"";
  appendJsonEscaped(json, g_storedVar.carParam[currentCarIndex].carName);
  json += "\",\"current\":{";

  snprintf(buf, sizeof(buf),
           "\"triggerPct\":%u,\"outputPct\":%u,\"brakePct\":%u,\"sensiHalfPct\":%u,"
           "\"vinMv\":%u,\"currentMa\":%u,\"releaseMode\":%u,\"currentSense\":%u",
           triggerPct,
           outputPct,
           brakePct,
           g_escVar.effectiveSensi_raw,
           g_escVar.Vin_mV,
           g_escVar.motorCurrent_mA,
           g_storedVar.carParam[currentCarIndex].quickBrakeEnabled,
           HAL_HasMotorCurrentSense() ? 1U : 0U);
  json += buf;

  json += ",\"statusSlots\":[";
  for (uint8_t i = 0; i < STATUS_SLOTS; i++) {
    if (i > 0) json += ",";
    json += String(normalizeStatusSlotForUi(g_storedVar.statusSlot[i]));
  }
  json += "]}";
}

static String buildTelemetryStatusPayload(const char* message) {
  TelemetryStatus status;
  telemetryGetStatus(&status);

  String json;
  json.reserve(768);
  json += "{\"ok\":true";
  if (message != nullptr && message[0] != '\0') {
    json += ",\"message\":\"";
    appendJsonEscaped(json, message);
    json += "\"";
  }
  appendTelemetryStatusFields(json, status);
  json += "}";
  return json;
}

static String buildTelemetryLivePayload(uint32_t afterSeq, size_t limit) {
  static TelemetrySample samples[256];
  static TelemetryEvent events[TELEMETRY_EVENT_BUFFER_CAPACITY];
  TelemetryStatus status;
  bool truncated = false;
  bool hasMore = false;
  bool eventsTruncated = false;
  size_t copied = telemetryCopySamplesAfter(afterSeq, samples, limit, &truncated, &hasMore, &status);
  size_t eventCopied = telemetryCopyEvents(events, TELEMETRY_EVENT_BUFFER_CAPACITY, &eventsTruncated, nullptr);

  String json;
  json.reserve(1600 + (copied * 92U) + (eventCopied * 180U));
  json += "{\"ok\":true";
  appendTelemetryStatusFields(json, status);
  json += ",\"truncated\":";
  json += truncated ? "true" : "false";
  json += ",\"hasMore\":";
  json += hasMore ? "true" : "false";
  json += ",\"returned\":";
  json += String((unsigned long)copied);
  json += ",\"eventTruncated\":";
  json += eventsTruncated ? "true" : "false";
  json += ",\"events\":[";
  for (size_t i = 0; i < eventCopied; i++) {
    if (i > 0) {
      json += ",";
    }
    appendTelemetryEventJson(json, events[i]);
  }
  json += "],\"samples\":[";

  for (size_t i = 0; i < copied; i++) {
    const TelemetrySample& s = samples[i];
    if (i > 0) {
      json += ",";
    }
    json += "[";
    json += String((unsigned long)s.seq);
    json += ",";
    json += String((unsigned long)s.t_ms);
    json += ",";
    json += String((unsigned int)s.trigger_pct);
    json += ",";
    json += String((unsigned int)s.output_pct);
    json += ",";
    json += String((unsigned int)s.vin_mV);
    json += ",";
    json += String((unsigned int)s.current_mA);
    json += ",";
    json += String((unsigned int)s.brake_pct);
    json += ",";
    json += String((unsigned int)s.sensi_halfPct);
    json += ",";
    json += String((unsigned int)s.carIndex);
    json += ",";
    json += String((unsigned int)s.releaseMode);
    json += ",";
    json += String((unsigned int)s.flags);
    json += "]";
  }

  json += "]}";
  return json;
}

static String buildTelemetryConfigSnapshotJson() {
  TelemetryConfigSnapshot snapshot;
  TelemetryStatus status;
  bool hasSnapshot = telemetryGetConfigSnapshot(&snapshot);
  telemetryGetStatus(&status);
  if (!hasSnapshot) {
    snapshot.storedVar = g_storedVar;
    snapshot.antiSpinStepMs = g_antiSpinStepMs;
    snapshot.encoderInvertEnabled = g_encoderInvertEnabled;
    snapshot.adcVoltageRange_mV = g_adcVoltageRange_mV;
  }
  uint8_t activeCarIndex = status.hasData ? status.sessionStartCarIndex : (uint8_t)g_carSel;
  if (activeCarIndex >= CAR_MAX_COUNT) {
    activeCarIndex = (uint8_t)g_storedVar.selectedCarNumber;
  }
  return buildTelemetryConfigSummaryJsonFromSnapshot(snapshot, activeCarIndex);
}

static void sendSerialLengthPrefixedPayload(const String& payload) {
  Serial.print(payload.length());
  Serial.print('\n');
  Serial.print(payload);
  Serial.flush();
}

static void handleTelemetryStatus() {
  g_wifiServer->send(200, "application/json", buildTelemetryStatusPayload(""));
}

static void handleTelemetryStart() {
  if (!telemetryStartLogging(&g_storedVar,
                             g_antiSpinStepMs,
                             g_encoderInvertEnabled,
                             g_adcVoltageRange_mV,
                             (uint8_t)g_carSel)) {
    g_wifiServer->send(500, "application/json", "{\"ok\":false,\"error\":\"Telemetry start failed\"}");
    return;
  }

  g_wifiServer->send(200, "application/json", buildTelemetryStatusPayload("Telemetry logging started"));
}

static void handleTelemetryStop() {
  telemetryStopLogging();
  g_wifiServer->send(200, "application/json", buildTelemetryStatusPayload("Telemetry logging stopped"));
}

static void handleTelemetryClear() {
  telemetryStopLogging();
  telemetryClear();
  g_wifiServer->send(200, "application/json", buildTelemetryStatusPayload("Telemetry buffer cleared"));
}

static void handleTelemetryLive() {
  uint32_t afterSeq = getTelemetryArgU32("after", 0U);
  size_t limit = getTelemetryLiveLimit();
  g_wifiServer->send(200, "application/json", buildTelemetryLivePayload(afterSeq, limit));
}

static void handleTelemetryExportCsv() {
  TelemetryStatus status;
  telemetryGetStatus(&status);
  if (!status.hasData) {
    g_wifiServer->send(404, "application/json", "{\"ok\":false,\"error\":\"No telemetry data available\"}");
    return;
  }

  TelemetryConfigSnapshot snapshot;
  bool hasSnapshot = telemetryGetConfigSnapshot(&snapshot);
  if (!hasSnapshot) {
    snapshot.storedVar = g_storedVar;
    snapshot.antiSpinStepMs = g_antiSpinStepMs;
    snapshot.encoderInvertEnabled = g_encoderInvertEnabled;
    snapshot.adcVoltageRange_mV = g_adcVoltageRange_mV;
  }

  g_wifiServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  g_wifiServer->sendHeader("Content-Disposition", "attachment; filename=\"espeed32-telemetry.csv\"");
  g_wifiServer->send(200, "text/csv; charset=utf-8", "");

  g_wifiServer->sendContent("seq,t_ms,car_index,car_name,trigger_pct,output_pct,brake_pct,sensi_pct,vin_mV,current_mA,release_mode,brake_button,trigger_releasing,in_release_zone,release_active,current_sense\n");

  static TelemetrySample batch[96];
  uint32_t afterSeq = 0;

  while (true) {
    TelemetryStatus batchStatus;
    bool truncated = false;
    bool hasMore = false;
    size_t copied = telemetryCopySamplesAfter(afterSeq, batch, 96U, &truncated, &hasMore, &batchStatus);
    if (copied == 0) {
      break;
    }

    String chunk;
    chunk.reserve(copied * 96U);
    for (size_t i = 0; i < copied; i++) {
      const TelemetrySample& s = batch[i];
      char sensiBuf[12];
      char row[192];
      formatHalfPercentValue(s.sensi_halfPct, sensiBuf, sizeof(sensiBuf));
      snprintf(row, sizeof(row),
               "%lu,%lu,%u,%s,%u,%u,%u,%s,%u,%u,%s,%u,%u,%u,%u,%u\n",
               (unsigned long)s.seq,
               (unsigned long)s.t_ms,
               (unsigned int)s.carIndex,
               snapshot.storedVar.carParam[s.carIndex].carName,
               (unsigned int)s.trigger_pct,
               (unsigned int)s.output_pct,
               (unsigned int)s.brake_pct,
               sensiBuf,
               (unsigned int)s.vin_mV,
               (unsigned int)s.current_mA,
               getTelemetryReleaseModeLabel(s.releaseMode),
               (s.flags & TELEMETRY_FLAG_BRAKE_BUTTON) ? 1U : 0U,
               (s.flags & TELEMETRY_FLAG_TRIGGER_RELEASING) ? 1U : 0U,
               (s.flags & TELEMETRY_FLAG_IN_RELEASE_ZONE) ? 1U : 0U,
               (s.flags & TELEMETRY_FLAG_RELEASE_ACTIVE) ? 1U : 0U,
               (s.flags & TELEMETRY_FLAG_CURRENT_SENSE) ? 1U : 0U);
      chunk += row;
      afterSeq = s.seq;
    }

    g_wifiServer->sendContent(chunk);
    if (!hasMore) {
      break;
    }
  }

  g_wifiServer->sendContent("");
}

static void handleTelemetryExportJson() {
  TelemetryStatus status;
  telemetryGetStatus(&status);
  if (!status.hasData) {
    g_wifiServer->send(404, "application/json", "{\"ok\":false,\"error\":\"No telemetry data available\"}");
    return;
  }

  TelemetryConfigSnapshot snapshot;
  bool hasSnapshot = telemetryGetConfigSnapshot(&snapshot);
  if (!hasSnapshot) {
    snapshot.storedVar = g_storedVar;
    snapshot.antiSpinStepMs = g_antiSpinStepMs;
    snapshot.encoderInvertEnabled = g_encoderInvertEnabled;
    snapshot.adcVoltageRange_mV = g_adcVoltageRange_mV;
  }

  char versionBuf[8];
  snprintf(versionBuf, sizeof(versionBuf), "%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);

  g_wifiServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  g_wifiServer->sendHeader("Content-Disposition", "attachment; filename=\"espeed32-telemetry.json\"");
  g_wifiServer->send(200, "application/json; charset=utf-8", "");

  static TelemetryEvent events[TELEMETRY_EVENT_BUFFER_CAPACITY];
  bool eventsTruncated = false;
  size_t eventCount = telemetryCopyEvents(events, TELEMETRY_EVENT_BUFFER_CAPACITY, &eventsTruncated, nullptr);

  String head;
  head.reserve(1800 + (eventCount * 180U));
  head += "{";
  head += "\"ok\":true,\"format\":\"espeed32-telemetry-v1\",\"deviceId\":\"";
  appendJsonEscaped(head, g_wifiSuffix);
  head += "\",\"firmware\":\"";
  appendJsonEscaped(head, versionBuf);
  head += "\",\"loggingActive\":";
  head += status.loggingActive ? "true" : "false";
  head += ",\"sampleRateMs\":";
  head += String(status.sampleRateMs);
  head += ",\"capacity\":";
  head += String(status.capacity);
  head += ",\"storedCount\":";
  head += String(status.storedCount);
  head += ",\"sessionId\":";
  head += String((unsigned long)status.sessionId);
  head += ",\"sessionStartMs\":";
  head += String((unsigned long)status.sessionStartMs);
  head += ",\"sessionStartCarIndex\":";
  head += String(status.sessionStartCarIndex);
  head += ",\"configAtStart\":";
  head += buildTelemetryConfigSummaryJsonFromSnapshot(snapshot, status.sessionStartCarIndex);
  head += ",\"eventsTruncated\":";
  head += eventsTruncated ? "true" : "false";
  head += ",\"events\":[";
  for (size_t i = 0; i < eventCount; i++) {
    if (i > 0) {
      head += ",";
    }
    appendTelemetryEventJson(head, events[i]);
  }
  head += "],\"samples\":[";
  g_wifiServer->sendContent(head);

  static TelemetrySample batch[64];
  uint32_t afterSeq = 0;
  bool first = true;

  while (true) {
    TelemetryStatus batchStatus;
    bool truncated = false;
    bool hasMore = false;
    size_t copied = telemetryCopySamplesAfter(afterSeq, batch, 64U, &truncated, &hasMore, &batchStatus);
    if (copied == 0) {
      break;
    }

    String chunk;
    chunk.reserve(copied * 128U);
    for (size_t i = 0; i < copied; i++) {
      const TelemetrySample& s = batch[i];
      if (!first) {
        chunk += ",";
      }
      first = false;
      chunk += "{\"seq\":";
      chunk += String((unsigned long)s.seq);
      chunk += ",\"tMs\":";
      chunk += String((unsigned long)s.t_ms);
      chunk += ",\"triggerPct\":";
      chunk += String((unsigned int)s.trigger_pct);
      chunk += ",\"outputPct\":";
      chunk += String((unsigned int)s.output_pct);
      chunk += ",\"vinMv\":";
      chunk += String((unsigned int)s.vin_mV);
      chunk += ",\"currentMa\":";
      chunk += String((unsigned int)s.current_mA);
      chunk += ",\"brakePct\":";
      chunk += String((unsigned int)s.brake_pct);
      chunk += ",\"sensiHalfPct\":";
      chunk += String((unsigned int)s.sensi_halfPct);
      chunk += ",\"carIndex\":";
      chunk += String((unsigned int)s.carIndex);
      chunk += ",\"releaseMode\":";
      chunk += String((unsigned int)s.releaseMode);
      chunk += ",\"flags\":";
      chunk += String((unsigned int)s.flags);
      chunk += "}";
      afterSeq = s.seq;
    }

    g_wifiServer->sendContent(chunk);
    if (!hasMore) {
      break;
    }
  }

  g_wifiServer->sendContent("]}");
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
      !streamHtmlFromSpiffs("/docs/nl/index.html") &&
      !streamHtmlFromSpiffs("/docs/es/index.html") &&
      !streamHtmlFromSpiffs("/docs/de/index.html") &&
      !streamHtmlFromSpiffs("/docs/it/index.html")) {
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

static void handleDocsNl() {
  if (!streamHtmlFromSpiffs("/docs/nl/index.html")) {
    sendDocsMissing("/docs/nl/index.html");
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

static void handleDocsIt() {
  if (!streamHtmlFromSpiffs("/docs/it/index.html")) {
    sendDocsMissing("/docs/it/index.html");
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

  uint16_t tempAntiSpinStep = g_antiSpinStepMs;
  uint16_t tempEncoderInvert = g_encoderInvertEnabled ? 1 : 0;
  uint16_t tempAdcVoltageRange = g_adcVoltageRange_mV;
  if (parseAndValidateJson(g_uploadBuffer, &tempVar, &tempAntiSpinStep, &tempEncoderInvert, &tempAdcVoltageRange, &errorMsg)) {
    g_storedVar = tempVar;
    g_antiSpinStepMs = tempAntiSpinStep;
    g_encoderInvertEnabled = tempEncoderInvert ? 1 : 0;
    applyAdcVoltageRangeMilliVolts(tempAdcVoltageRange);
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
 * @brief Handle OTA upload - streams firmware (.bin) or SPIFFS image (.bin) directly to flash
 */
static void handleOtaUpload() {
  HTTPUpload& upload = g_wifiServer->upload();
  char msgStr[32];

  if (upload.status == UPLOAD_FILE_START) {
    String uri = g_wifiServer->uri();
    g_otaTargetSpiffs = (uri == "/ota-spiffs");
    g_otaTotal = upload.totalSize;  /* May be 0 if browser doesn't send content-length */
    g_otaWritten = 0;
    g_otaInProgress = true;
    g_otaSessionOk = true;

    /* Show update in progress on OLED */
    obdFill(&g_obd, OBD_WHITE, 1);
    if (g_otaTargetSpiffs) {
      obdWriteString(&g_obd, 0, 8, 0, (char*)"SPIFFS Update", FONT_8x8, OBD_BLACK, 1);
    } else {
      obdWriteString(&g_obd, 0, 16, 0, (char*)"OTA Update", FONT_8x8, OBD_BLACK, 1);
    }
    obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Updating...", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, (char*)"Do not power off!", FONT_6x8, OBD_BLACK, 1);

    if (g_otaTargetSpiffs && g_spiffsMounted) {
      SPIFFS.end();
      g_spiffsMounted = false;
    }

#if defined(U_SPIFFS)
    int updateCommand = g_otaTargetSpiffs ? U_SPIFFS : U_FLASH;
#else
    int updateCommand = U_FLASH;
    if (g_otaTargetSpiffs) {
      g_otaSessionOk = false;
      g_otaInProgress = false;
      return;
    }
#endif

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateCommand)) {
      g_otaSessionOk = false;
      g_otaInProgress = false;
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!g_otaSessionOk) {
      return;
    }
    if (Update.write(upload.buf, upload.currentSize) == upload.currentSize) {
      g_otaWritten += upload.currentSize;
      /* Update progress on OLED */
      sprintf(msgStr, "%u KB", (unsigned int)(g_otaWritten / 1024));
      obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, msgStr, FONT_8x8, OBD_BLACK, 1);
    } else {
      g_otaSessionOk = false;
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    bool otaOk = g_otaSessionOk && Update.end(true);
    if (otaOk) {
      obdFill(&g_obd, OBD_WHITE, 1);
      if (g_otaTargetSpiffs) {
        obdWriteString(&g_obd, 0, 0, 24, (char*)"SPIFFS OK!", FONT_12x16, OBD_BLACK, 1);
      } else {
        obdWriteString(&g_obd, 0, 8, 24, (char*)"OTA OK!", FONT_12x16, OBD_BLACK, 1);
      }
    } else {
      obdFill(&g_obd, OBD_WHITE, 1);
      if (g_otaTargetSpiffs) {
        obdWriteString(&g_obd, 0, 0, 24, (char*)"SPIFFS FAIL!", FONT_12x16, OBD_BLACK, 1);
      } else {
        obdWriteString(&g_obd, 0, 0, 24, (char*)"OTA FAIL!", FONT_12x16, OBD_BLACK, 1);
      }
    }
    g_otaInProgress = false;
  }
}

/**
 * @brief Handle OTA completion response - called after upload finishes
 */
static void handleOta() {
  bool otaOk = g_otaSessionOk && !Update.hasError();
  if (!otaOk) {
    if (g_otaTargetSpiffs && !g_spiffsMounted) {
      g_spiffsMounted = SPIFFS.begin(false);
    }
    if (g_otaTargetSpiffs) {
      g_wifiServer->send(400, "text/plain", "Error: SPIFFS update failed");
    } else {
      g_wifiServer->send(400, "text/plain", "Error: firmware update failed");
    }
  } else {
    if (g_otaTargetSpiffs) {
      g_wifiServer->send(200, "text/plain", "OK - SPIFFS updated! Restarting...");
    } else {
      g_wifiServer->send(200, "text/plain", "OK - Firmware updated! Restarting...");
    }
    delay(1000);
    ESP.restart();
  }
}

static const uint8_t WIFI_QR_VERSION = 3;
static const uint8_t WIFI_QR_SIZE = 29;
static const uint8_t WIFI_QR_DATA_CODEWORDS = 55;
static const uint8_t WIFI_QR_ECC_CODEWORDS = 15;
static const uint8_t WIFI_QR_SCALE = 2;
static const uint8_t WIFI_QR_QUIET_ZONE = 1;

static inline bool qrGetBit(uint16_t value, uint8_t bit) {
  return ((value >> bit) & 1U) != 0;
}

static void qrSetFunctionModule(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                int row, int col, bool isBlack) {
  if (row < 0 || row >= WIFI_QR_SIZE || col < 0 || col >= WIFI_QR_SIZE) {
    return;
  }
  modules[row][col] = isBlack ? 1 : 0;
  functionModules[row][col] = 1;
}

static void qrReserveModule(uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                            int row, int col) {
  if (row < 0 || row >= WIFI_QR_SIZE || col < 0 || col >= WIFI_QR_SIZE) {
    return;
  }
  functionModules[row][col] = 1;
}

static void qrDrawFinderPattern(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                int top, int left) {
  for (int dy = -1; dy <= 7; dy++) {
    for (int dx = -1; dx <= 7; dx++) {
      int row = top + dy;
      int col = left + dx;
      if (row < 0 || row >= WIFI_QR_SIZE || col < 0 || col >= WIFI_QR_SIZE) {
        continue;
      }
      bool isBlack = (dx >= 0 && dx <= 6 && dy >= 0 && dy <= 6 &&
                      (dx == 0 || dx == 6 || dy == 0 || dy == 6 ||
                       (dx >= 2 && dx <= 4 && dy >= 2 && dy <= 4)));
      qrSetFunctionModule(modules, functionModules, row, col, isBlack);
    }
  }
}

static void qrDrawAlignmentPattern(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                   uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                   int centerRow, int centerCol) {
  for (int dy = -2; dy <= 2; dy++) {
    for (int dx = -2; dx <= 2; dx++) {
      bool isBlack = max(abs(dx), abs(dy)) != 1;
      qrSetFunctionModule(modules, functionModules, centerRow + dy, centerCol + dx, isBlack);
    }
  }
}

static void qrDrawFunctionPatterns(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                                   uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  memset(modules, 0, WIFI_QR_SIZE * WIFI_QR_SIZE);
  memset(functionModules, 0, WIFI_QR_SIZE * WIFI_QR_SIZE);

  qrDrawFinderPattern(modules, functionModules, 0, 0);
  qrDrawFinderPattern(modules, functionModules, 0, WIFI_QR_SIZE - 7);
  qrDrawFinderPattern(modules, functionModules, WIFI_QR_SIZE - 7, 0);
  qrDrawAlignmentPattern(modules, functionModules, 22, 22);

  for (int i = 8; i < WIFI_QR_SIZE - 8; i++) {
    bool isBlack = (i % 2) == 0;
    qrSetFunctionModule(modules, functionModules, 6, i, isBlack);
    qrSetFunctionModule(modules, functionModules, i, 6, isBlack);
  }

  for (int i = 0; i < 9; i++) {
    qrReserveModule(functionModules, 8, i);
    qrReserveModule(functionModules, i, 8);
  }
  for (int i = 0; i < 8; i++) {
    qrReserveModule(functionModules, 8, WIFI_QR_SIZE - 1 - i);
  }
  for (int i = 0; i < 7; i++) {
    qrReserveModule(functionModules, WIFI_QR_SIZE - 1 - i, 8);
  }

  qrSetFunctionModule(modules, functionModules, WIFI_QR_SIZE - 8, 8, true);
}

static bool qrAppendBits(uint8_t out[WIFI_QR_DATA_CODEWORDS], int* bitLen,
                         uint32_t value, uint8_t bitCount) {
  if (bitLen == nullptr) {
    return false;
  }
  if ((*bitLen + bitCount) > (WIFI_QR_DATA_CODEWORDS * 8)) {
    return false;
  }
  for (int i = bitCount - 1; i >= 0; i--) {
    uint8_t bit = (uint8_t)((value >> i) & 1U);
    out[*bitLen >> 3] |= bit << (7 - (*bitLen & 7));
    (*bitLen)++;
  }
  return true;
}

static uint8_t qrGfMultiply(uint8_t x, uint8_t y) {
  uint16_t a = x;
  uint16_t b = y;
  uint16_t z = 0;
  while (b != 0) {
    if (b & 1U) {
      z ^= a;
    }
    b >>= 1;
    a <<= 1;
    if (a & 0x100U) {
      a ^= 0x11DU;
    }
  }
  return (uint8_t)z;
}

static void qrBuildGenerator(uint8_t out[WIFI_QR_ECC_CODEWORDS]) {
  memset(out, 0, WIFI_QR_ECC_CODEWORDS);
  out[WIFI_QR_ECC_CODEWORDS - 1] = 1;
  uint8_t root = 1;
  for (uint8_t i = 0; i < WIFI_QR_ECC_CODEWORDS; i++) {
    for (uint8_t j = 0; j < WIFI_QR_ECC_CODEWORDS; j++) {
      out[j] = qrGfMultiply(out[j], root);
      if (j + 1 < WIFI_QR_ECC_CODEWORDS) {
        out[j] ^= out[j + 1];
      }
    }
    root = qrGfMultiply(root, 0x02);
  }
}

static void qrComputeRemainder(const uint8_t data[WIFI_QR_DATA_CODEWORDS],
                               const uint8_t generator[WIFI_QR_ECC_CODEWORDS],
                               uint8_t remainder[WIFI_QR_ECC_CODEWORDS]) {
  memset(remainder, 0, WIFI_QR_ECC_CODEWORDS);
  for (uint8_t i = 0; i < WIFI_QR_DATA_CODEWORDS; i++) {
    uint8_t factor = data[i] ^ remainder[0];
    for (uint8_t j = 0; j < WIFI_QR_ECC_CODEWORDS - 1; j++) {
      remainder[j] = remainder[j + 1] ^ qrGfMultiply(generator[j], factor);
    }
    remainder[WIFI_QR_ECC_CODEWORDS - 1] =
      qrGfMultiply(generator[WIFI_QR_ECC_CODEWORDS - 1], factor);
  }
}

static void qrPlaceCodewords(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                             const uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                             const uint8_t codewords[WIFI_QR_DATA_CODEWORDS + WIFI_QR_ECC_CODEWORDS]) {
  int bitIndex = 0;
  bool upwards = true;

  for (int right = WIFI_QR_SIZE - 1; right >= 1; right -= 2) {
    if (right == 6) {
      right--;
    }
    for (int i = 0; i < WIFI_QR_SIZE; i++) {
      int row = upwards ? (WIFI_QR_SIZE - 1 - i) : i;
      for (int j = 0; j < 2; j++) {
        int col = right - j;
        if (functionModules[row][col]) {
          continue;
        }
        bool bit = false;
        if (bitIndex < ((WIFI_QR_DATA_CODEWORDS + WIFI_QR_ECC_CODEWORDS) * 8)) {
          bit = ((codewords[bitIndex >> 3] >> (7 - (bitIndex & 7))) & 1U) != 0;
        }
        modules[row][col] = bit ? 1 : 0;
        bitIndex++;
      }
    }
    upwards = !upwards;
  }
}

static bool qrMaskApplies(uint8_t mask, int row, int col) {
  switch (mask) {
    case 0: return ((row + col) % 2) == 0;
    case 1: return (row % 2) == 0;
    case 2: return (col % 3) == 0;
    case 3: return ((row + col) % 3) == 0;
    case 4: return (((row / 2) + (col / 3)) % 2) == 0;
    case 5: return (((row * col) % 2) + ((row * col) % 3)) == 0;
    case 6: return ((((row * col) % 2) + ((row * col) % 3)) % 2) == 0;
    case 7: return ((((row + col) % 2) + ((row * col) % 3)) % 2) == 0;
    default: return false;
  }
}

static void qrApplyMask(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                        const uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE],
                        uint8_t mask) {
  for (int row = 0; row < WIFI_QR_SIZE; row++) {
    for (int col = 0; col < WIFI_QR_SIZE; col++) {
      if (!functionModules[row][col] && qrMaskApplies(mask, row, col)) {
        modules[row][col] ^= 1;
      }
    }
  }
}

static uint16_t qrGetFormatBits(uint8_t mask) {
  uint16_t data = (1U << 3) | mask;  /* ECC level L = 01 */
  uint16_t rem = data << 10;
  for (int bit = 14; bit >= 10; bit--) {
    if (((rem >> bit) & 1U) != 0) {
      rem ^= (uint16_t)(0x537U << (bit - 10));
    }
  }
  return (uint16_t)(((data << 10) | rem) ^ 0x5412U);
}

static void qrDrawFormatBits(uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE], uint8_t mask) {
  uint16_t formatBits = qrGetFormatBits(mask);

  for (uint8_t i = 0; i <= 5; i++) {
    modules[i][8] = qrGetBit(formatBits, i) ? 1 : 0;
  }
  modules[7][8] = qrGetBit(formatBits, 6) ? 1 : 0;
  modules[8][8] = qrGetBit(formatBits, 7) ? 1 : 0;
  modules[8][7] = qrGetBit(formatBits, 8) ? 1 : 0;
  for (uint8_t i = 9; i < 15; i++) {
    modules[8][14 - i] = qrGetBit(formatBits, i) ? 1 : 0;
  }

  for (uint8_t i = 0; i < 8; i++) {
    modules[8][WIFI_QR_SIZE - 1 - i] = qrGetBit(formatBits, i) ? 1 : 0;
  }
  for (uint8_t i = 8; i < 15; i++) {
    modules[WIFI_QR_SIZE - 15 + i][8] = qrGetBit(formatBits, i) ? 1 : 0;
  }
  modules[WIFI_QR_SIZE - 8][8] = 1;
}

static int qrPenaltyRule1(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  int penalty = 0;

  for (int row = 0; row < WIFI_QR_SIZE; row++) {
    int runLength = 1;
    uint8_t runColor = modules[row][0];
    for (int col = 1; col < WIFI_QR_SIZE; col++) {
      if (modules[row][col] == runColor) {
        runLength++;
      } else {
        if (runLength >= 5) {
          penalty += runLength - 2;
        }
        runColor = modules[row][col];
        runLength = 1;
      }
    }
    if (runLength >= 5) {
      penalty += runLength - 2;
    }
  }

  for (int col = 0; col < WIFI_QR_SIZE; col++) {
    int runLength = 1;
    uint8_t runColor = modules[0][col];
    for (int row = 1; row < WIFI_QR_SIZE; row++) {
      if (modules[row][col] == runColor) {
        runLength++;
      } else {
        if (runLength >= 5) {
          penalty += runLength - 2;
        }
        runColor = modules[row][col];
        runLength = 1;
      }
    }
    if (runLength >= 5) {
      penalty += runLength - 2;
    }
  }

  return penalty;
}

static int qrPenaltyRule2(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  int penalty = 0;
  for (int row = 0; row < WIFI_QR_SIZE - 1; row++) {
    for (int col = 0; col < WIFI_QR_SIZE - 1; col++) {
      uint8_t color = modules[row][col];
      if (color == modules[row][col + 1] &&
          color == modules[row + 1][col] &&
          color == modules[row + 1][col + 1]) {
        penalty += 3;
      }
    }
  }
  return penalty;
}

static bool qrFinderPenaltyPattern(const uint8_t line[11]) {
  static const uint8_t PATTERN_A[11] = {1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0};
  static const uint8_t PATTERN_B[11] = {0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1};
  bool matchA = true;
  bool matchB = true;
  for (uint8_t i = 0; i < 11; i++) {
    if (line[i] != PATTERN_A[i]) {
      matchA = false;
    }
    if (line[i] != PATTERN_B[i]) {
      matchB = false;
    }
  }
  return matchA || matchB;
}

static int qrPenaltyRule3(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  int penalty = 0;
  uint8_t line[11];

  for (int row = 0; row < WIFI_QR_SIZE; row++) {
    for (int col = 0; col <= WIFI_QR_SIZE - 11; col++) {
      for (uint8_t i = 0; i < 11; i++) {
        line[i] = modules[row][col + i];
      }
      if (qrFinderPenaltyPattern(line)) {
        penalty += 40;
      }
    }
  }

  for (int col = 0; col < WIFI_QR_SIZE; col++) {
    for (int row = 0; row <= WIFI_QR_SIZE - 11; row++) {
      for (uint8_t i = 0; i < 11; i++) {
        line[i] = modules[row + i][col];
      }
      if (qrFinderPenaltyPattern(line)) {
        penalty += 40;
      }
    }
  }

  return penalty;
}

static int qrPenaltyRule4(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  int darkCount = 0;
  for (int row = 0; row < WIFI_QR_SIZE; row++) {
    for (int col = 0; col < WIFI_QR_SIZE; col++) {
      darkCount += modules[row][col] ? 1 : 0;
    }
  }

  const int totalModules = WIFI_QR_SIZE * WIFI_QR_SIZE;
  int k = abs(darkCount * 20 - totalModules * 10) / totalModules;
  return k * 10;
}

static int qrGetPenaltyScore(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  return qrPenaltyRule1(modules) +
         qrPenaltyRule2(modules) +
         qrPenaltyRule3(modules) +
         qrPenaltyRule4(modules);
}

static bool buildWiFiQrMatrix(const char* payload,
                              uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  if (payload == nullptr) {
    return false;
  }

  size_t payloadLen = strlen(payload);
  if (payloadLen == 0 || payloadLen > 53) {
    return false;
  }

  uint8_t dataCodewords[WIFI_QR_DATA_CODEWORDS];
  uint8_t generator[WIFI_QR_ECC_CODEWORDS];
  uint8_t remainder[WIFI_QR_ECC_CODEWORDS];
  uint8_t allCodewords[WIFI_QR_DATA_CODEWORDS + WIFI_QR_ECC_CODEWORDS];
  uint8_t functionModules[WIFI_QR_SIZE][WIFI_QR_SIZE];

  memset(dataCodewords, 0, sizeof(dataCodewords));
  int bitLen = 0;
  if (!qrAppendBits(dataCodewords, &bitLen, 0x4, 4)) {
    return false;
  }
  if (!qrAppendBits(dataCodewords, &bitLen, (uint32_t)payloadLen, 8)) {
    return false;
  }
  for (size_t i = 0; i < payloadLen; i++) {
    if (!qrAppendBits(dataCodewords, &bitLen, (uint8_t)payload[i], 8)) {
      return false;
    }
  }

  int capacityBits = WIFI_QR_DATA_CODEWORDS * 8;
  int terminatorBits = min(4, capacityBits - bitLen);
  if (terminatorBits > 0 && !qrAppendBits(dataCodewords, &bitLen, 0, (uint8_t)terminatorBits)) {
    return false;
  }

  int zeroPadBits = (8 - (bitLen & 7)) & 7;
  if (zeroPadBits > 0 && !qrAppendBits(dataCodewords, &bitLen, 0, (uint8_t)zeroPadBits)) {
    return false;
  }

  bool useAltPad = false;
  while (bitLen < capacityBits) {
    dataCodewords[bitLen >> 3] = useAltPad ? 0x11 : 0xEC;
    useAltPad = !useAltPad;
    bitLen += 8;
  }

  qrBuildGenerator(generator);
  qrComputeRemainder(dataCodewords, generator, remainder);

  memcpy(allCodewords, dataCodewords, WIFI_QR_DATA_CODEWORDS);
  memcpy(allCodewords + WIFI_QR_DATA_CODEWORDS, remainder, WIFI_QR_ECC_CODEWORDS);

  qrDrawFunctionPatterns(modules, functionModules);
  qrPlaceCodewords(modules, functionModules, allCodewords);

  uint8_t bestMask = 0;
  int bestPenalty = 0x7FFFFFFF;
  uint8_t trial[WIFI_QR_SIZE][WIFI_QR_SIZE];
  for (uint8_t mask = 0; mask < 8; mask++) {
    memcpy(trial, modules, sizeof(trial));
    qrApplyMask(trial, functionModules, mask);
    qrDrawFormatBits(trial, mask);
    int penalty = qrGetPenaltyScore(trial);
    if (penalty < bestPenalty) {
      bestPenalty = penalty;
      bestMask = mask;
    }
  }

  qrApplyMask(modules, functionModules, bestMask);
  qrDrawFormatBits(modules, bestMask);
  return true;
}

static bool buildWiFiQrPayload(const char* ssid, const char* password,
                               char out[64]) {
  if (ssid == nullptr || password == nullptr || out == nullptr) {
    return false;
  }
  int written = snprintf(out, 64, "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);
  return written > 0 && written < 64;
}

static void drawWiFiQrMatrix(const uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE]) {
  const int totalPixels = (WIFI_QR_SIZE + (2 * WIFI_QR_QUIET_ZONE)) * WIFI_QR_SCALE;
  const int originX = (OLED_WIDTH - totalPixels) / 2;
  const int originY = (OLED_HEIGHT - totalPixels) / 2;

  obdFill(&g_obd, OBD_WHITE, 1);
  for (int row = 0; row < WIFI_QR_SIZE; row++) {
    for (int col = 0; col < WIFI_QR_SIZE; col++) {
      if (!modules[row][col]) {
        continue;
      }
      int x0 = originX + (WIFI_QR_QUIET_ZONE + col) * WIFI_QR_SCALE;
      int y0 = originY + (WIFI_QR_QUIET_ZONE + row) * WIFI_QR_SCALE;
      for (uint8_t dy = 0; dy < WIFI_QR_SCALE; dy++) {
        obdDrawLine(&g_obd, x0, y0 + dy, x0 + WIFI_QR_SCALE - 1, y0 + dy, OBD_BLACK, 1);
      }
    }
  }
}

static void buildWifiSuffixIfNeeded() {
  if (g_wifiSuffix[0] != '\0') {
    return;
  }
  uint8_t wifiMac[6] = {0};
  if (readMacAddress(ESP_MAC_WIFI_STA, wifiMac)) {
    sprintf(g_wifiSuffix, "%02X%02X", wifiMac[4], wifiMac[5]);
  } else {
    uint64_t mac = ESP.getEfuseMac();
    sprintf(g_wifiSuffix, "%02X%02X", (uint8_t)(mac >> 8), (uint8_t)(mac));
  }
}

void getWiFiPortalSsid(char* out, size_t outLen) {
  if (out == nullptr || outLen == 0) {
    return;
  }
  buildWifiSuffixIfNeeded();
  snprintf(out, outLen, "%s_%s", WIFI_SSID, g_wifiSuffix);
}

IPAddress getWiFiPortalIP() {
  return WiFi.softAPIP();
}

static void registerWebRoutes() {
  if (g_wifiServer == nullptr) {
    return;
  }

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
  g_wifiServer->on("/api/telemetry/status", HTTP_GET, handleTelemetryStatus);
  g_wifiServer->on("/api/telemetry/live", HTTP_GET, handleTelemetryLive);
  g_wifiServer->on("/api/telemetry/start", HTTP_POST, handleTelemetryStart);
  g_wifiServer->on("/api/telemetry/stop", HTTP_POST, handleTelemetryStop);
  g_wifiServer->on("/api/telemetry/clear", HTTP_POST, handleTelemetryClear);
  g_wifiServer->on("/api/telemetry/export.csv", HTTP_GET, handleTelemetryExportCsv);
  g_wifiServer->on("/api/telemetry/export.json", HTTP_GET, handleTelemetryExportJson);
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
  g_wifiServer->on("/docs/nl", HTTP_GET, handleDocsNl);
  g_wifiServer->on("/docs/nl/", HTTP_GET, handleDocsNl);
  g_wifiServer->on("/docs/nl/index.html", HTTP_GET, handleDocsNl);
  g_wifiServer->on("/docs/es", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/es/", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/es/index.html", HTTP_GET, handleDocsEs);
  g_wifiServer->on("/docs/de", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/de/", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/de/index.html", HTTP_GET, handleDocsDe);
  g_wifiServer->on("/docs/it", HTTP_GET, handleDocsIt);
  g_wifiServer->on("/docs/it/", HTTP_GET, handleDocsIt);
  g_wifiServer->on("/docs/it/index.html", HTTP_GET, handleDocsIt);
  g_wifiServer->on("/docs/assets/curve_examples.png", HTTP_GET, handleDocsCurveAsset);
  g_wifiServer->on("/docs/assets/trig_cal.png", HTTP_GET, handleDocsTriggerAsset);
  g_wifiServer->on("/restore", HTTP_POST, handleRestore, handleRestoreUpload);
  g_wifiServer->on("/ota", HTTP_POST, handleOta, handleOtaUpload);
  g_wifiServer->on("/ota-spiffs", HTTP_POST, handleOta, handleOtaUpload);
}

bool isWiFiPortalActive() {
  return g_wifiServer != nullptr;
}

bool isOtaInProgress() {
  return g_otaInProgress;
}

bool startWiFiPortal() {
  if (g_wifiServer != nullptr) {
    return true;
  }

  char ssid[20];
  getWiFiPortalSsid(ssid, sizeof(ssid));

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(ssid, WIFI_PASS, WIFI_AP_CHANNEL, 0, WIFI_MAX_CONNECTIONS)) {
    WiFi.mode(WIFI_OFF);
    return false;
  }
  delay(100);

  g_spiffsMounted = SPIFFS.begin(false);

  g_wifiServer = new WebServer(80);
  if (g_wifiServer == nullptr) {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    if (g_spiffsMounted) {
      SPIFFS.end();
      g_spiffsMounted = false;
    }
    return false;
  }

  registerWebRoutes();
  g_wifiServer->begin();
  return true;
}

void serviceWiFiPortal() {
  if (g_wifiServer != nullptr) {
    g_wifiServer->handleClient();
  }
}

void serviceConnectivityPortal() {
  serviceWiFiPortal();
  serviceUsbSerialCommands();
}

void stopWiFiPortal() {
  if (g_otaInProgress) {
    return;
  }

  telemetryStopLogging();

  if (g_wifiServer != nullptr) {
    g_wifiServer->stop();
    delete g_wifiServer;
    g_wifiServer = nullptr;
  }

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  if (g_spiffsMounted) {
    SPIFFS.end();
    g_spiffsMounted = false;
  }
}


/**
 * @brief Main WiFi backup/restore screen - called from settings menu
 * @details Starts WiFi AP, runs HTTP server, shows info on OLED.
 *          Returns when user presses encoder button or brake button.
 */
void showWiFiPortalScreen() {
  char msgStr[32];
  bool wasAlreadyActive = isWiFiPortalActive();

  if (!startWiFiPortal()) {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, (char*)"WiFi start failed", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"Press to exit", FONT_8x8, OBD_BLACK, 1);
    while (!g_rotaryEncoder.isEncoderButtonClicked()) {
      if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
        break;
      }
      vTaskDelay(1);
    }
    obdFill(&g_obd, OBD_WHITE, 1);
    return;
  }

  char ssid[20];
  getWiFiPortalSsid(ssid, sizeof(ssid));
  IPAddress ip = getWiFiPortalIP();

  /* Display WiFi info on OLED (FONT_6x8 = 21 chars/line) */
  obdFill(&g_obd, OBD_WHITE, 1);
  const char* wifiTitle = "WiFi info";
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
    serviceConnectivityPortal();

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

  if (!wasAlreadyActive) {
    stopWiFiPortal();
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}

void showWiFiQrScreen() {
  bool wasAlreadyActive = isWiFiPortalActive();

  if (!startWiFiPortal()) {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, (char*)"WiFi start failed", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"Press to exit", FONT_8x8, OBD_BLACK, 1);
    while (!g_rotaryEncoder.isEncoderButtonClicked()) {
      if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
        break;
      }
      vTaskDelay(1);
    }
    obdFill(&g_obd, OBD_WHITE, 1);
    return;
  }

  char ssid[20];
  char payload[64];
  uint8_t modules[WIFI_QR_SIZE][WIFI_QR_SIZE];

  getWiFiPortalSsid(ssid, sizeof(ssid));
  if (!buildWiFiQrPayload(ssid, WIFI_PASS, payload) ||
      !buildWiFiQrMatrix(payload, modules)) {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, (char*)"QR build failed", FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"Press to exit", FONT_8x8, OBD_BLACK, 1);
    while (!g_rotaryEncoder.isEncoderButtonClicked()) {
      if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        delay(BUTTON_SHORT_PRESS_DEBOUNCE_MS);
        break;
      }
      vTaskDelay(1);
    }
    if (!wasAlreadyActive) {
      stopWiFiPortal();
    }
    obdFill(&g_obd, OBD_WHITE, 1);
    return;
  }

  drawWiFiQrMatrix(modules);

  while (true) {
    serviceConnectivityPortal();

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

  if (!wasAlreadyActive) {
    stopWiFiPortal();
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
    serviceConnectivityPortal();

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

    vTaskDelay(1);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}
