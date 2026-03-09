/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include "slot_ESC.h"
#include "screensaver_config.h"  /* Personal screensaver configuration (git-ignored) */
#include "connectivity_portal.h"
#include <esp_mac.h>

/* Version defined in slot_ESC.h */

/*********************************************************************************************************************/
/*                                                   Language Strings                                                */
/*********************************************************************************************************************/

/* Menu item names in different languages: [language][item] */
const char* MENU_NAMES[][11] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE", "PWM_F", "B_KNP", "H-Brems", "GRENSE", "INNSTILL", "STATS", "*BIL*"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "Q-Brake", "LIMIT", "SETTINGS", "STATS", "*CAR*"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL", "PWM_F", "B_BTN", "Q-Brake", "CHOKE1", "SETTINGS", "STATS", "*CAR*"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "Q-Brake", "CHOKE", "SETTINGS", "STATS", "*CAR*"}
};

/* Settings menu item names: [language][item] */
const char* SETTINGS_MENU_NAMES[][9] = {
  /* NOR */ {"STROM", "SKJERM", "LYD", "WIFI", "USB INFO", "NULLSTILL", "TEST", "INFO", "TILBAKE"},
  /* ENG */ {"POWER", "DISPLAY", "SOUND", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* CS  */ {"POWER", "DISPLAY", "SOUND", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* ACD */ {"POWER", "DISPLAY", "SOUND", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"}
};

/* Power submenu item names: [language][item] */
const char* POWER_MENU_NAMES[][5] = {
  /* NOR */ {"SKJSP", "DVALE", "DYP SOV", "OPPSTART", "TILBAKE"},
  /* ENG */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"},
  /* CS  */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"},
  /* ACD */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"}
};

/* Display submenu item names: [language][item] */
const char* DISPLAY_MENU_NAMES[][6] = {
  /* NOR */ {"RACEMODUS", "SPRAK", "STYL", "SKRIFTSTRL", "STATUSLINJE", "TILBAKE"},
  /* ENG */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"},
  /* CS  */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"},
  /* ACD */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"}
};

/* Race mode parameter labels: [language][param] */
const char* RACE_LABELS[][4] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE"}
};

/* Car menu option labels: [language][option] */
const char* CAR_MENU_OPTIONS[][5] = {
  /* NOR */ {"VELG", "NAVNGI", "RACESWP", "KOPIER", "NULLSTILL"},
  /* ENG */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* CS  */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* ACD */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"}
};

/* View mode value labels: [language][mode] */
const char* VIEW_MODE_LABELS[][3] = {
  /* NOR */ {"AV", "FULL", "ENKEL"},
  /* ENG */ {"OFF", "FULL", "SIMPLE"},
  /* CS  */ {"OFF", "FULL", "SIMPLE"},
  /* ACD */ {"OFF", "FULL", "SIMPLE"}
};

/* Sound submenu item names: [language][item] — BOOT, RACE, BACK */
const char* SOUND_MENU_NAMES[][3] = {
  /* NOR */ {"OPPSTART", "RACEMODE", "TILBAKE"},
  /* ENG */ {"BOOT", "RACE", "BACK"},
  /* CS  */ {"BOOT", "RACE", "BACK"},
  /* ACD */ {"BOOT", "RACE", "BACK"}
};

/* ON/OFF labels: [language][state] */
const char* ON_OFF_LABELS[][2] = {
  /* NOR */ {"AV", "PA"},
  /* ENG */ {"OFF", "ON"},
  /* CS  */ {"OFF", "ON"},
  /* ACD */ {"OFF", "ON"}
};

/* Language labels: [language_code] */
const char* LANG_LABELS[] = {"NOR", "ENG", "CS", "ACD"};

/* Text case style labels: [language][case_style] */
const char* TEXT_CASE_LABELS[][2] = {
  /* NOR */ {"UPPER", "Pascal"},
  /* ENG */ {"UPPER", "Pascal"},
  /* CS  */ {"UPPER", "Pascal"},
  /* ACD */ {"UPPER", "Pascal"}
};

/* Font size labels: [language][size] */
const char* FONT_SIZE_LABELS[][2] = {
  /* NOR */ {"STOR", "liten"},
  /* ENG */ {"LARGE", "small"},
  /* CS  */ {"LARGE", "small"},
  /* ACD */ {"LARGE", "small"}
};

/*********************************************************************************************************************/
/*                                    Pascal Case String Arrays                                                      */
/*********************************************************************************************************************/

/* Menu item names - Pascal Case: [language][item] */
const char* MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve", "Pwm_F", "B_Knp", "H-Brems", "Grense", "Innstill", "Stats", "*Bil*"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "Q-Brake", "Limit", "Settings", "Stats", "*Car*"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil", "Pwm_F", "B_Btn", "Q-Brake", "Choke1", "Settings", "Stats", "*Car*"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "Q-Brake", "Choke", "Settings", "Stats", "*Car*"}
};

/* Settings menu item names - Pascal Case: [language][item] */
const char* SETTINGS_MENU_NAMES_PASCAL[][9] = {
  /* NOR */ {"Strom", "Skjerm", "Lyd", "Wifi", "Usb info", "Nullstill", "Test", "Info", "Tilbake"},
  /* ENG */ {"Power", "Display", "Sound", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* CS  */ {"Power", "Display", "Sound", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* ACD */ {"Power", "Display", "Sound", "Wifi", "Usb info", "Reset", "Test", "About", "Back"}
};

/* Power submenu item names - Pascal Case: [language][item] */
const char* POWER_MENU_NAMES_PASCAL[][5] = {
  /* NOR */ {"Skjsp", "Dvale", "Dyp Sov", "Oppstart", "Tilbake"},
  /* ENG */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"},
  /* CS  */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"},
  /* ACD */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"}
};

/* Display submenu item names - Pascal Case: [language][item] */
const char* DISPLAY_MENU_NAMES_PASCAL[][6] = {
  /* NOR */ {"Racemodus", "Sprak", "Styl", "Skriftstrl", "Statuslinje", "Tilbake"},
  /* ENG */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"},
  /* CS  */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"},
  /* ACD */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"}
};

/* Race mode parameter labels - Pascal Case: [language][param] */
const char* RACE_LABELS_PASCAL[][4] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve"}
};

/* Car menu option labels - Pascal Case: [language][option] */
const char* CAR_MENU_OPTIONS_PASCAL[][5] = {
  /* NOR */ {"Velg", "Navngi", "Raceswp", "Kopier", "Nullstill"},
  /* ENG */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* CS  */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* ACD */ {"Select", "Rename", "Raceswp", "Copy", "Reset"}
};

/* View mode value labels - Pascal Case: [language][mode] */
const char* VIEW_MODE_LABELS_PASCAL[][3] = {
  /* NOR */ {"Av", "Full", "Enkel"},
  /* ENG */ {"Off", "Full", "Simple"},
  /* CS  */ {"Off", "Full", "Simple"},
  /* ACD */ {"Off", "Full", "Simple"}
};

/* Sound submenu item names - Pascal Case: [language][item] */
const char* SOUND_MENU_NAMES_PASCAL[][3] = {
  /* NOR */ {"Oppstart", "Racemode", "Tilbake"},
  /* ENG */ {"Boot", "Race", "Back"},
  /* CS  */ {"Boot", "Race", "Back"},
  /* ACD */ {"Boot", "Race", "Back"}
};

/* ON/OFF labels - Pascal Case: [language][state] */
const char* ON_OFF_LABELS_PASCAL[][2] = {
  /* NOR */ {"Av", "Pa"},
  /* ENG */ {"Off", "On"},
  /* CS  */ {"Off", "On"},
  /* ACD */ {"Off", "On"}
};

/* BACK button labels - UPPER CASE: [language] */
const char* BACK_LABELS[] = {
  /* NOR */ "TILBAKE",
  /* ENG */ "BACK",
  /* CS  */ "BACK",
  /* ACD */ "BACK"
};

/* BACK button labels - Pascal Case: [language] */
const char* BACK_LABELS_PASCAL[] = {
  /* NOR */ "Tilbake",
  /* ENG */ "Back",
  /* CS  */ "Back",
  /* ACD */ "Back"
};

/* UI strings for car selection/copy/rename/reset screens: [language] */
const char* STR_SELECT_CAR[] = { "-VELG BIL-", "-SELECT THE CAR-", "-SELECT THE CAR-", "-SELECT THE CAR-" };
const char* STR_COPY_FROM[] = { "-KOPIER FRA:-", "-COPY FROM:-", "-COPY FROM:-", "-COPY FROM:-" };
const char* STR_COPY_TO[] = { "-KOPIER TIL:-", "-COPY TO:-", "-COPY TO:-", "-COPY TO:-" };
const char* STR_ALL[] = { "ALLE", "ALL", "ALL", "ALL" };
const char* STR_COPIED_ALL[] = { "KOPIERT!", "COPIED ALL", "COPIED ALL", "COPIED ALL" };
const char* STR_COPIED[] = { "KOPIERT!", "COPIED!", "COPIED!", "COPIED!" };
const char* STR_RENAME_CAR[] = { "-GI NYTT NAVN-", "-RENAME THE CAR-", "-RENAME THE CAR-", "-RENAME THE CAR-" };
const char* STR_CONFIRM[] = { "-TRYKK OK FOR GODTA-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-" };
const char* STR_LIMITER[] = { " - BEGRENS - ", " - LIMITER - ", " - CHOKE1 - ", " - CHOKE - " };
const char* STR_CALIBRATION[] = { "KALIBRERING", "CALIBRATION", "CALIBRATION", "CALIBRATION" };
const char* STR_PRESS_RELEASE[] = { "trykk/slipp gass", "press/releas throttle", "press/releas throttle", "press/releas throttle" };  /* max 21 chars (FONT_6x8) */
const char* STR_PUSH_DONE[] = { " trykk nar ferdig ", " push when done ", " push when done ", " push when done " };

/*********************************************************************************************************************/
/*                                                   Global Variables                                                */
/*********************************************************************************************************************/

/* FreeRTOS Task Handles */
TaskHandle_t Task1;
TaskHandle_t Task2;

/* State Machine */
static StateMachine_enum g_currState = INIT;

/* Display Components */
#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

OBDISP g_obd;         /* OLED display instance */
char msgStr[50];      /* Display message buffer */

/* Car Selection */
uint16_t g_carSel;    /* Currently selected car model index */

/* Rotary Encoder */
AiEsp32RotaryEncoder g_rotaryEncoder = AiEsp32RotaryEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN, ENCODER_VCC_PIN, ENCODER_STEPS);

static uint8_t g_encoderMainSelector = 1;             /* Main menu item selector */
static uint8_t g_encoderSecondarySelector = 0;        /* Secondary value selector */
static uint16_t *g_encoderSelectedValuePtr = NULL;    /* Pointer to currently selected value */
static uint16_t g_originalValueBeforeEdit = 0;        /* Store original value when entering VALUE_SELECTION (for cancel) */
static bool g_isEditingCarSelection = false;          /* Flag to prevent g_carSel update during CAR edit */

/* Stored Variables (EEPROM/Preferences) */
StoredVar_type g_storedVar;

/*********************************************************************************************************************/
/*                                            Helper Functions for Text Case                                         */
/*********************************************************************************************************************/

/* Get menu name with current text case style */
inline const char* getMenuName(uint8_t lang, uint8_t item) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? MENU_NAMES_PASCAL[lang][item] : MENU_NAMES[lang][item];
}

/* Get settings menu name with current text case style */
inline const char* getSettingsMenuName(uint8_t lang, uint8_t item) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? SETTINGS_MENU_NAMES_PASCAL[lang][item] : SETTINGS_MENU_NAMES[lang][item];
}

/* Get power submenu name with current text case style */
inline const char* getPowerMenuName(uint8_t lang, uint8_t item) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? POWER_MENU_NAMES_PASCAL[lang][item] : POWER_MENU_NAMES[lang][item];
}

/* Get display submenu name with current text case style */
inline const char* getDisplayMenuName(uint8_t lang, uint8_t item) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? DISPLAY_MENU_NAMES_PASCAL[lang][item] : DISPLAY_MENU_NAMES[lang][item];
}

/* Get race label with current text case style */
inline const char* getRaceLabel(uint8_t lang, uint8_t param) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? RACE_LABELS_PASCAL[lang][param] : RACE_LABELS[lang][param];
}

/* Get car menu option with current text case style */
inline const char* getCarMenuOption(uint8_t lang, uint8_t option) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? CAR_MENU_OPTIONS_PASCAL[lang][option] : CAR_MENU_OPTIONS[lang][option];
}

/* Get view mode label with current text case style */
inline const char* getViewModeLabel(uint8_t lang, uint8_t mode) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? VIEW_MODE_LABELS_PASCAL[lang][mode] : VIEW_MODE_LABELS[lang][mode];
}

/* Get sound submenu item name with current text case style */
inline const char* getSoundMenuName(uint8_t lang, uint8_t item) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? SOUND_MENU_NAMES_PASCAL[lang][item] : SOUND_MENU_NAMES[lang][item];
}

/* Get ON/OFF label with current text case style */
inline const char* getOnOffLabel(uint8_t lang, uint8_t state) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? ON_OFF_LABELS_PASCAL[lang][state] : ON_OFF_LABELS[lang][state];
}

/* Get BACK label with current text case style */
inline const char* getBackLabel(uint8_t lang) {
  return (g_storedVar.textCase == TEXT_CASE_PASCAL) ? BACK_LABELS_PASCAL[lang] : BACK_LABELS[lang];
}

/* Get number of visible menu lines based on font size */
inline uint8_t getMenuLines() {
  return (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? 5 : 3;
}

/* ESC Runtime Variables */
ESC_type g_escVar {
  .outputSpeed_pct = 0,
  .trigger_raw = 0,
  .trigger_norm = 0,
  .encoderPos = 1,
  .Vin_mV = 0,
  .lapCount = 0,
  .bestLapTime_ms = 0,
  .lapStartTime_ms = 0
};

/* Menu Structures */
Menu_type g_mainMenu {
  .lines = 3
};

Menu_type g_settingsMenu {
  .lines = 3
};

Menu_type g_carMenu {
  .lines = 3
};

/* Preferences (NVM storage) */
Preferences g_pref;

/* UI Timing */
static uint32_t g_lastEncoderInteraction = 0;  /* Timestamp of last encoder interaction for display power saving */

/* Menu Navigation State */
static bool g_inSettingsMenu = false;  /* Track if we're currently in the settings submenu */
static bool g_forceRaceRedraw = false; /* Force race mode display to redraw */
static bool g_escapeToMain = false;    /* Set by any submenu long press → cascade-breaks to RUNNING for race mode toggle */
static uint16_t g_wifiTimedMinutes = 5;       /* Runtime-only default for timed WiFi activation */
static bool g_wifiTimedActive = false;        /* True when background WiFi should auto-stop on deadline */
static uint32_t g_wifiTimedStopAtMs = 0;      /* millis() deadline for auto-stop */

/* Long press tracking shared across all submenus (only one active at a time) */
static uint32_t g_lpRaceStart = 0;
static bool g_lpRaceActive = false;
static void serviceTimedWiFiPortal();
static void showPowerSave();
static void showPowerSave(uint32_t inactivityStartMs);
static void showDeepSleep();

/**
 * @brief Detect long press on encoder button (shared across all submenus).
 * @return true once per long press when threshold is exceeded.
 */
static bool checkRaceModeEscape() {
  serviceTimedWiFiPortal();

  if (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED) {
    if (!g_lpRaceActive) { g_lpRaceStart = millis(); g_lpRaceActive = true; }
    if (millis() - g_lpRaceStart > BUTTON_LONG_PRESS_MS) {
      g_lpRaceActive = false;
      return true;
    }
  } else {
    g_lpRaceActive = false;
  }
  return false;
}

/**
 * @brief Service inactivity-based power transitions for UI loops.
 * @details Applies auto power save/deep sleep in all menus except race mode.
 * @param lastInteraction Pointer to loop-local last interaction timestamp.
 * @param screensaverActive Pointer to loop-local screensaver flag (optional).
 * @return true if power-save was entered and returned (caller should redraw), false otherwise.
 */
static bool serviceIdlePowerTransitions(uint32_t* lastInteraction, bool* screensaverActive) {
  if (lastInteraction == NULL) return false;

  if (g_storedVar.screensaverTimeout == 0) return false;

  uint32_t now = millis();

  /* OTA safety lock: do not enter sleep/deep sleep while upload is active. */
  if (isOtaInProgress()) {
    *lastInteraction = now;
    if (screensaverActive != NULL) *screensaverActive = false;
    return false;
  }

  uint32_t idleMs = now - *lastInteraction;
  uint32_t screensaverMs = (uint32_t)g_storedVar.screensaverTimeout * 1000UL;

  if (g_storedVar.deepSleepTimeout > 0 &&
      idleMs > screensaverMs + ((uint32_t)g_storedVar.deepSleepTimeout * 60000UL)) {
    showDeepSleep();  /* Never returns */
  }

  if (g_storedVar.powerSaveTimeout > 0 &&
      idleMs > screensaverMs + ((uint32_t)g_storedVar.powerSaveTimeout * 60000UL)) {
    showPowerSave(*lastInteraction);
    *lastInteraction = millis();
    if (screensaverActive != NULL) *screensaverActive = false;
    return true;
  }

  return false;
}

/**
 * @brief Consume wake-up input so it doesn't also trigger menu actions.
 * @details Prevents one wake press from being interpreted as BACK/OK in nested menus.
 * @param wakeTriggered True when current loop woke from screensaver.
 * @return true if caller should skip the rest of this iteration.
 */
static bool consumeScreensaverWakeInput(bool wakeTriggered) {
  if (!wakeTriggered) return false;

  g_lastEncoderInteraction = millis();
  g_rotaryEncoder.isEncoderButtonClicked();  /* consume pending click edge */

  uint32_t guardStart = millis();
  while (digitalRead(BUTT_PIN) == BUTTON_PRESSED &&
         (uint32_t)(millis() - guardStart) < BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
    vTaskDelay(1);
  }
  return true;
}

/**
 * @brief Toggle race/list view mode and reset encoder for new mode.
 * Called from RUNNING when g_escapeToMain is set, or on direct long press.
 * @param menuState Reference to RUNNING's menuState (reset to ITEM_SELECTION).
 * @param lastLongPressTime Reference to guard timestamp (prevent double-trigger).
 */
static void applyRaceModeToggle(MenuState_enum &menuState, uint32_t &lastLongPressTime) {
  if (g_storedVar.viewMode == VIEW_MODE_LIST && g_storedVar.raceViewMode != RACE_VIEW_OFF) {
    g_storedVar.viewMode = VIEW_MODE_GRID;
  } else if (g_storedVar.viewMode == VIEW_MODE_GRID) {
    g_storedVar.viewMode = VIEW_MODE_LIST;
  }
  obdFill(&g_obd, OBD_WHITE, 1);
  menuState = ITEM_SELECTION;
  if (g_storedVar.viewMode == VIEW_MODE_GRID) {
    uint8_t gridItems = (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE)
      ? (g_storedVar.gridCarSelectEnabled ? 3 : 2)
      : (g_storedVar.gridCarSelectEnabled ? 5 : 4);
    g_rotaryEncoder.setBoundaries(1, gridItems, false);
  } else {
    g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  }
  g_rotaryEncoder.reset(1);
  g_encoderMainSelector = 1;
  g_escVar.encoderPos = 1;
  lastLongPressTime = millis();
  saveEEPROM(g_storedVar);
  if (g_storedVar.soundRace) keySound();
  g_lastEncoderInteraction = millis();
  g_rotaryEncoder.isEncoderButtonClicked();  /* consume any pending click */
}

static void serviceTimedWiFiPortal() {
  serviceConnectivityPortal();

  if (!g_wifiTimedActive) {
    return;
  }

  if (!isWiFiPortalActive()) {
    g_wifiTimedActive = false;
    return;
  }

  if ((int32_t)(millis() - g_wifiTimedStopAtMs) >= 0) {
    if (isOtaInProgress()) {
      /* Keep portal alive during OTA and retry timed stop later. */
      g_wifiTimedStopAtMs = millis() + 1000UL;
      return;
    }
    stopWiFiPortal();
    g_wifiTimedActive = false;
  }
}

static void startTimedWiFiPortal(uint16_t minutes) {
  minutes = constrain(minutes, 1, 120);
  if (startWiFiPortal()) {
    g_wifiTimedMinutes = minutes;
    g_wifiTimedActive = true;
    g_wifiTimedStopAtMs = millis() + ((uint32_t)minutes * 60000UL);
  }
}

static void stopTimedWiFiPortal() {
  if (isOtaInProgress()) {
    return;
  }
  g_wifiTimedActive = false;
  if (isWiFiPortalActive()) {
    stopWiFiPortal();
  }
}

/*********************************************************************************************************************/
/*                                                Function Prototypes                                                */
/*********************************************************************************************************************/
void IRAM_ATTR readEncoderISR();
static bool resetConfirm(const char* label);
static void doResetCar();
static void showPowerSave();
static void showPowerSave(uint32_t inactivityStartMs);
static void showDeepSleep();
static void showSleepSettings();
static void showDeepSleepSettings();
static void showSoundSettings();
static void showWiFiSettings();
static void showPowerSettings();
static void showDisplaySettings();
static void showAboutScreen();
static void showSelfTest();
static void serviceTimedWiFiPortal();
static void startTimedWiFiPortal(uint16_t minutes);
static void stopTimedWiFiPortal();
void initDisplayMenuItems();

/*********************************************************************************************************************/
/*                                                Setup Function                                                     */
/*********************************************************************************************************************/

/**
 * @brief Main setup function - initializes hardware and creates FreeRTOS tasks
 */
void setup() {
  /* Pin and Serial Setup */
  HAL_PinSetup();

  /* HalfBridge & Hardware Setup */
  HalfBridge_Setup();

  /* Create FreeRTOS Tasks */
  /* Task 1: UI and state machine (low priority, core 0) */
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function */
    "Task1",     /* Task name */
    10000,       /* Stack size */
    NULL,        /* Parameters */
    1,           /* Priority */
    &Task1,      /* Task handle */
    0);          /* Core 0 */
    
  /* Task 2: Trigger reading and motor control (high priority, core 1) */
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function */
    "Task2",     /* Task name */
    10000,       /* Stack size */
    NULL,        /* Parameters */
    2,           /* Priority */
    &Task2,      /* Task handle */
    1);          /* Core 1 */
}


/*********************************************************************************************************************/
/*                                                FreeRTOS Tasks                                                     */
/*********************************************************************************************************************/

/**
 * @brief Task 1: UI and State Machine
 * @details Manages OLED display, rotary encoder, and main state machine
 * @note Runs on Core 0 with lower priority
 */
void Task1code(void *pvParameters) {
  for (;;) {
    StateMachine_enum prevState = g_currState;
    static uint16_t prevFreqPWM = 0;
    static MenuState_enum menuState = ITEM_SELECTION;
    static uint8_t swMajVer, swMinVer, storedVarVersion;

    /* Read motor current (voltage is read exclusively in Task2 to avoid ADC contention) */
    g_escVar.motorCurrent_mA = HAL_ReadMotorCurrent();
    serviceTimedWiFiPortal();

    /* Update selected car if initialization complete */
    if (g_currState != INIT) {
      g_carSel = g_storedVar.selectedCarNumber;
    }

    /* Task 1 state machine */
    switch (g_currState) {
      case INIT:

        g_pref.begin("stored_var", false); /* Open the "stored" namespace in read/write mode. If it doesn't exist, it creates it */

        if (g_pref.isKey("stored_var_ver") && g_pref.isKey("sw_maj_ver") && g_pref.isKey("sw_min_ver") && g_pref.isKey("user_param")) /* If all keys exists, then check their value */
        {
          /* Get the values of the sw version */
          swMajVer = g_pref.getUChar("sw_maj_ver");
          swMinVer = g_pref.getUChar("sw_min_ver");
          storedVarVersion = g_pref.getUChar("stored_var_ver");

          if ((storedVarVersion == STORED_VAR_VERSION) ) /* If the storedVariable version keys is equal to the STORED_VAR MACRO, then the stored param are already initialized woh the proper format*/
          {
            g_pref.getBytes("user_param", &g_storedVar, sizeof(g_storedVar)); /* Get the value of the stored user_param */
            initMenuItems();                                                  /* init menu items with EEPROM stored variables */

            /* If calibration reset was requested from settings, go straight to calibration */
            if (g_pref.getBool("force_calib", false)) {
              g_pref.putBool("force_calib", false);
              g_pref.end();
              g_currState = CALIBRATION;
              g_storedVar.minTrigger_raw = MAX_INT16;
              g_storedVar.maxTrigger_raw = MIN_INT16;
              if (g_storedVar.soundBoot) {
                calibSound();
              }
              initDisplayAndEncoder();
              obdFill(&g_obd, OBD_WHITE, 1);
              break;
            }

            /* If button is pressed at startup, go to CALIBRATION state */
            if (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED)
            {
              g_currState = CALIBRATION;      /* Go to CALIBRATION state */
              /* Reset Min and Max to the opposite side, in order to have effective calibration */
              g_storedVar.minTrigger_raw = MAX_INT16;
              g_storedVar.maxTrigger_raw = MIN_INT16;
              if (g_storedVar.soundBoot) {
                calibSound();             /* Play calibration sound */
              }
              initDisplayAndEncoder();  /* init and clear OLED and Encoder */

              /* Wait until button is released, then go to CALIBRATION state */
              while (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED)
              {
                showScreenPreCalibration();
              }
              
              obdFill(&g_obd, OBD_WHITE, 1); /* Clear OLED */
            }
            else if (digitalRead(BUTT_PIN) == BUTTON_PRESSED)
            {
              /* Hold brake button at startup → run self-test */
              g_currState = WELCOME;
              g_carSel = g_storedVar.selectedCarNumber;
              initDisplayAndEncoder();
              showSelfTest();
            }
            else  /* If button is NOT pressed at startup, go to RUNNING state */
            {
              g_currState = WELCOME;                    /* Go to WELCOME state */
              g_carSel = g_storedVar.selectedCarNumber; /* now it is safe to address the proper car */
              initDisplayAndEncoder();  /* init and clear OLED and Encoder */
              if (g_storedVar.soundBoot) {
                onSound();                /* Play ON sound */
              }
            }

            g_pref.end(); /* Close the namespace */
            break;        /* Break the switch case: if code reaches here, it means that the stored user param and sw versions are OK */
          }
        }

        /* If the code reaches here it means that:
        - the sw version keys are not present --> stored var are not initialized
        - the sw version stored are not up to date --> stored var are initialized but might be outdated

        Calibration values are NOT stored, go to CALIBRATION state */
        initDisplayAndEncoder();  /* init and clear OLED and Encoder */
                              
        g_pref.clear();           /* Clear all the keys in this namespace */
        /* Store the correct SW version */
        g_pref.putUChar("sw_maj_ver", SW_MAJOR_VERSION);
        g_pref.putUChar("sw_min_ver", SW_MINOR_VERSION);
        
        g_pref.putUChar("stored_var_ver", STORED_VAR_VERSION);

        initStoredVariables();  /* Initialize stored variables with default values */

        /* Reset Min and Max to the opposite side, in order to have effective calibration */
        g_storedVar.minTrigger_raw = MAX_INT16;
        g_storedVar.maxTrigger_raw = MIN_INT16;
        if (g_storedVar.soundBoot) {
          calibSound();                   /* Play calibration sound */
        }
        g_currState = CALIBRATION;      /* Go to CALIBRATION state */
        obdFill(&g_obd, OBD_WHITE, 1); /* Clear OLED */
        /* Press and release button to go to CALIBRATION state */
        while (!g_rotaryEncoder.isEncoderButtonClicked()) /* Loop until button is pressed */
        {
          showScreenNoEEPROM();
        }

        break;


      case CALIBRATION:
        /* Read Throttle */
        throttleCalibration(g_escVar.trigger_raw);    /* trigger raw is continuously read on task2 */
        showScreenCalibration(g_escVar.trigger_raw);  /* Show calibration screen */
        /* Exit calibration if button is presseded */
        if (g_rotaryEncoder.isEncoderButtonClicked())  /* exit calibration and save calibration data to EEPROM */
        {
          if (g_storedVar.soundBoot) {
            offSound();
          }
          initMenuItems();  /* Init Menu Items */
          saveEEPROM(g_storedVar);  /* Save modified calibration values to EEPROM */
          HalfBridge_Enable();    /* Enable HalfBridge */
          g_currState = WELCOME;  /* Go to WELCOME state */
        }
        break;


      case WELCOME:
        /* This WELCOME state is only used to show the SW versions on the Screen, but the controller is actually fully operational:
            - Calibration is OKAY (either just done, or using stored values)
            - User param are OKAY (either just initialized to default, or using stored values)
            - Trigger is being read on Task 2

          The user trigger input is being elaborated and the correct speed (PWM) output is being produced for the whole duration of the WELCOME state */

        if (g_storedVar.startupDelay > 0) {
          showScreenWelcome();    /* Show welcome screen */
          delay(g_storedVar.startupDelay * STARTUP_DELAY_STEP_MS);  /* Configurable startup delay */
        }
        g_currState = RUNNING;  /* Go to RUNNING state */
        break;


      case RUNNING: /* when the global variable State is in RUNNING the Task2 will elaborate the trigger to produce the PWM out */

        /* Set encoder boundaries based on view mode (on first entry) */
        static bool encoderBoundariesSet = false;
        if (!encoderBoundariesSet) {
          /* Force LIST mode if race view is OFF */
          if (g_storedVar.viewMode == VIEW_MODE_GRID && g_storedVar.raceViewMode == RACE_VIEW_OFF) {
            g_storedVar.viewMode = VIEW_MODE_LIST;
          }

          if (g_storedVar.viewMode == VIEW_MODE_GRID) {
            uint8_t gridItems;
            if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
              gridItems = g_storedVar.gridCarSelectEnabled ? 3 : 2;  /* SIMPLE: BRAKE, SENSI, CAR (optional) */
            } else {
              gridItems = g_storedVar.gridCarSelectEnabled ? 5 : 4;  /* FULL: BRAKE, SENSI, ANTIS, CURVE, CAR (optional) */
            }
            g_rotaryEncoder.setBoundaries(1, gridItems, false);
          } else {
            g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);  /* Normal menu items */
          }
          encoderBoundariesSet = true;
        }

        /* Handle race mode toggle: either from a submenu escape or direct long press */
        static uint32_t lastLongPressTime = 0;

        if (g_escapeToMain) {
          /* Return from submenu long press → toggle race mode immediately */
          g_escapeToMain = false;
          applyRaceModeToggle(menuState, lastLongPressTime);
        } else {
          /* Direct long press detection in RUNNING */
          static uint32_t buttonPressStartTime = 0;
          static bool buttonWasPressed = false;
          if (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED) {
            if (!buttonWasPressed) {
              buttonPressStartTime = millis();
              buttonWasPressed = true;
            }
            if (millis() - buttonPressStartTime > BUTTON_LONG_PRESS_MS &&
                millis() - lastLongPressTime > BUTTON_LONG_PRESS_MS) {
              applyRaceModeToggle(menuState, lastLongPressTime);
            }
          } else {
            buttonWasPressed = false;
          }
        }

        /* Check for brake button press - acts as "back" in edit mode */
        static bool brakeButtonWasPressedInMenu = false;
        static uint32_t lastBrakeButtonPressTime = 0;

        /* In GRID mode: brake button exits edit mode (in addition to reducing brake) */
        if (g_storedVar.viewMode == VIEW_MODE_GRID && menuState == VALUE_SELECTION && digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
          if (!brakeButtonWasPressedInMenu && millis() - lastBrakeButtonPressTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
            brakeButtonWasPressedInMenu = true;
            lastBrakeButtonPressTime = millis();

            /* Restore original value (cancel changes) */
            if (g_encoderSelectedValuePtr != NULL) {
              *g_encoderSelectedValuePtr = g_originalValueBeforeEdit;
              g_encoderSelectedValuePtr = NULL;
            }

            menuState = ITEM_SELECTION;
            g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);

            uint8_t gridItems;
            if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
              gridItems = g_storedVar.gridCarSelectEnabled ? 3 : 2;
            } else {
              gridItems = g_storedVar.gridCarSelectEnabled ? 5 : 4;
            }
            g_rotaryEncoder.setBoundaries(1, gridItems, false);
            g_rotaryEncoder.reset(g_encoderMainSelector);
            g_escVar.encoderPos = g_encoderMainSelector;
            g_isEditingCarSelection = false;
            obdFill(&g_obd, OBD_WHITE, 1);
            g_lastEncoderInteraction = millis();
          }
        }
        else if (g_storedVar.viewMode == VIEW_MODE_LIST && digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
          if (!brakeButtonWasPressedInMenu && millis() - lastBrakeButtonPressTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
            brakeButtonWasPressedInMenu = true;
            lastBrakeButtonPressTime = millis();

            /* If in VALUE_SELECTION, cancel changes and go back to ITEM_SELECTION */
            if (menuState == VALUE_SELECTION) {
              /* Restore original value (cancel changes) */
              if (g_encoderSelectedValuePtr != NULL) {
                *g_encoderSelectedValuePtr = g_originalValueBeforeEdit;
                g_encoderSelectedValuePtr = NULL;  /* Clear pointer after use */
              }

              menuState = ITEM_SELECTION;
              g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
              g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
              g_rotaryEncoder.reset(g_encoderMainSelector);
              g_escVar.encoderPos = g_encoderMainSelector;
              /* Clear car editing flag */
              g_isEditingCarSelection = false;
              /* Do NOT save to EEPROM - we're canceling the change */
              obdFill(&g_obd, OBD_WHITE, 1);  /* Clear screen */
              g_lastEncoderInteraction = millis();
            }
            /* If already in ITEM_SELECTION, brake button doesn't do anything in main menu */
          }
        }
        /* Reset flag when button is released */
        if (digitalRead(BUTT_PIN) != BUTTON_PRESSED) {
          brakeButtonWasPressedInMenu = false;
        }

        /* Change menu state if encoder button is clicked (short press) */
        /* Only process if sufficient time has passed since last long press */
        if ((lastLongPressTime == 0 || millis() - lastLongPressTime > BUTTON_DEBOUNCE_AFTER_LONG_MS) &&
            g_rotaryEncoder.isEncoderButtonClicked())
        {
          /* If screensaver is active (timeout exceeded), just wake up - don't process menu action */
          if (g_storedVar.screensaverTimeout > 0 && millis() - g_lastEncoderInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
            g_lastEncoderInteraction = millis();  /* Wake from screensaver */
          } else {
            menuState = rotary_onButtonClick(menuState);  /* This function is called if the encoder is pressed
                                                             Take the current menu state and returns the next menu state */
            g_lastEncoderInteraction = millis();          /* Update last encoder interaction */
          }
        }

        /* Get encoder position if it was changed*/
        if (g_rotaryEncoder.encoderChanged()) 
        {
          g_escVar.encoderPos = g_rotaryEncoder.readEncoder();  /* Update the global ESC variable keeping track of the encoder value (position) */
          g_lastEncoderInteraction = millis();  /* Update last encoder interaction */
        }

        /* Change Encoder variable depending on menu state */
        if (menuState == ITEM_SELECTION) 
        {
          g_encoderMainSelector = g_escVar.encoderPos;  /* If in ITEM_SELECTION, update the encoder MainSelector with the encoder position */
        }
        if (menuState == VALUE_SELECTION)
        {
          g_encoderSecondarySelector = g_escVar.encoderPos;         /* If in ITEM_SELECTION, update the encoder SecondaryEncoder with the encoder position */
          uint16_t prevLanguage = g_storedVar.language;             /* Store previous language for change detection */
          *g_encoderSelectedValuePtr = g_encoderSecondarySelector;  /* Also update the value of the selected parameter */

          /* If language changed, reinitialize menu items with new language strings */
          if (g_storedVar.language != prevLanguage) {
            initMenuItems();
            obdFill(&g_obd, OBD_WHITE, 1);  /* Clear screen to prevent leftover text */
          }
        }

        /* Show Main Menu display */
        printMainMenu(menuState);
        if (g_storedVar.carParam[g_carSel].freqPWM != prevFreqPWM)  /* if PWM freq parameter is changed, update motor PWM */
        {
          prevFreqPWM = g_storedVar.carParam[g_carSel].freqPWM;
          ledcDetach(HB_IN_PIN);
          ledcDetach(HB_INH_PIN);
          uint16_t freqTmp = g_storedVar.carParam[g_carSel].freqPWM * 100;
          ledcAttachChannel(HB_IN_PIN, freqTmp, THR_PWM_RES_BIT, THR_IN_PWM_CHAN);
          ledcAttachChannel(HB_INH_PIN, freqTmp, THR_PWM_RES_BIT, THR_INH_PWM_CHAN);
        }
        if (g_escVar.outputSpeed_pct == 100) /* indicate 100% throttle also on the internal ESP32 LED*/
          digitalWrite(LED_BUILTIN, 1);
        else
          digitalWrite(LED_BUILTIN, 0);
        break;


      case FAULT:
        /* FAULT state, not currently used */
        break;


      default:
        /* Default case, do nothing */
        break;
    }

    if (g_currState != prevState) /* Every time FSM machine change state */
      obdFill(&g_obd, OBD_WHITE, 1);
  }
}

/**
 * @brief Task 2: Trigger Reading and Motor Control
 * @details Reads trigger input, applies throttle curve and anti-spin, controls motor PWM
 * @note Runs on Core 1 with higher priority for real-time motor control
 */
void Task2code(void *pvParameters) {
  static unsigned long prevCallTime_uS = 0;
  static unsigned long currTrigger_raw = 0, prevTrigger_raw = 0;

  HalfBridge_Enable();

  for (;;) {
    unsigned long currCallTime_uS = micros();
    unsigned long deltaTime_uS = currCallTime_uS - prevCallTime_uS;
    
    /* Execute every ESC_PERIOD_US microseconds */
    if (deltaTime_uS > ESC_PERIOD_US) {
      prevCallTime_uS = currCallTime_uS;

      /* Read and filter trigger input */
      prevTrigger_raw = currTrigger_raw;
      currTrigger_raw = HAL_ReadTriggerRaw();
      g_escVar.trigger_raw = (prevTrigger_raw + currTrigger_raw) / 2;  /* Simple moving average filter */
      
      /* Normalize and apply deadband */
      g_escVar.trigger_norm = normalizeAndClamp(g_escVar.trigger_raw, g_storedVar.minTrigger_raw, 
                                                g_storedVar.maxTrigger_raw, THROTTLE_NORMALIZED, THROTTLE_REV);
      g_escVar.trigger_norm = addDeadBand(g_escVar.trigger_norm, 0, THROTTLE_NORMALIZED, THROTTLE_DEADBAND_NORM);
      
      /* Lap detection via track voltage dead spot */
      {
        static uint8_t lapState = 0;  /* 0=TRACKING, 1=GAP, 2=COOLDOWN */
        static uint32_t gapStartMs = 0;
        static uint32_t lapRegisteredMs = 0;
        uint32_t vinRaw = analogRead(AN_VIN_DIV);
        uint32_t vinMv = (ACD_VOLTAGE_RANGE_MVOLTS * vinRaw / ACD_RESOLUTION_STEPS)
                         * (RVIFBL + RVIFBH) / RVIFBL;

        /* Update display voltage with 8-sample moving average to filter ADC noise */
        static uint32_t vinFilter[8] = {0};
        static uint8_t vinFilterIdx = 0;
        vinFilter[vinFilterIdx] = vinMv;
        vinFilterIdx = (vinFilterIdx + 1) % 8;
        uint32_t vinSum = 0;
        for (uint8_t f = 0; f < 8; f++) vinSum += vinFilter[f];
        g_escVar.Vin_mV = (uint16_t)(vinSum / 8);
        uint32_t nowMs = millis();

        switch (lapState) {
          case 0: /* TRACKING */
            if (vinMv < LAP_VIN_THRESHOLD_MV) {
              gapStartMs = nowMs;
              lapState = 1;
            }
            break;
          case 1: /* GAP */
            if (vinMv >= LAP_VIN_THRESHOLD_MV) {
              if ((nowMs - gapStartMs) <= LAP_GAP_MAX_MS) {
                /* Valid dead spot crossing */
                if (g_escVar.lapStartTime_ms > 0) {
                  uint32_t lapTime = nowMs - g_escVar.lapStartTime_ms;
                  uint8_t idx = g_escVar.lapCount % LAP_MAX_COUNT;
                  g_escVar.lapTimes[idx] = lapTime;
                  if (g_escVar.lapCount == 0 || lapTime < g_escVar.bestLapTime_ms)
                    g_escVar.bestLapTime_ms = lapTime;
                  g_escVar.lapCount++;
                }
                g_escVar.lapStartTime_ms = nowMs;
                lapRegisteredMs = nowMs;
                lapState = 2;
              } else {
                lapState = 0; /* Too long = crash/stop */
              }
            } else if ((nowMs - gapStartMs) > LAP_GAP_MAX_MS) {
              lapState = 0; /* Still in gap and too long - abort */
            }
            break;
          case 2: /* COOLDOWN */
            if ((nowMs - lapRegisteredMs) >= LAP_MIN_TIME_MS) {
              lapState = 0;
            }
            break;
        }
      }

      /* Apply motor control (skip if in calibration or init state) */
      if (!(g_currState == CALIBRATION || g_currState == INIT)) {
        if (g_escVar.trigger_norm == 0) {
          /* Apply brake when trigger is released */
          /* Check if brake button is pressed - use alternate brake value */
          uint16_t effectiveBrake = g_storedVar.carParam[g_carSel].brake;
          if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
            /* Use brakeButtonReduction as alternate brake value (not a reduction) */
            effectiveBrake = g_storedVar.carParam[g_carSel].brakeButtonReduction;
          }
          HalfBridge_SetPwmDrag(0, effectiveBrake);
          g_escVar.outputSpeed_pct = 0;
          throttleAntiSpin3(0);  /* Keep anti-spin timer updated */
        } else {
          /* Check if quick brake zone is active */
          bool inQuickBrakeZone = false;
          if (g_storedVar.carParam[g_carSel].quickBrakeEnabled) {
            uint16_t qbThreshold_norm = (uint32_t)g_storedVar.carParam[g_carSel].quickBrakeThreshold
                                        * THROTTLE_NORMALIZED / 100;
            inQuickBrakeZone = (g_escVar.trigger_norm < qbThreshold_norm);
          }
          if (inQuickBrakeZone) {
            /* Quick brake zone: apply configured brake strength */
            HalfBridge_SetPwmDrag(0, g_storedVar.carParam[g_carSel].quickBrakeStrength);
            g_escVar.outputSpeed_pct = 0;
            throttleAntiSpin3(0);  /* Keep anti-spin timer updated */
          } else {
            /* Apply throttle curve and anti-spin */
            g_escVar.outputSpeed_pct = throttleCurve2(g_escVar.trigger_norm);
            g_escVar.outputSpeed_pct = throttleAntiSpin3(g_escVar.outputSpeed_pct);
            HalfBridge_SetPwmDrag(g_escVar.outputSpeed_pct, 0);
          }
          /* Update last interaction time to prevent screensaver while driving */
          g_lastEncoderInteraction = millis();
        }
      }
    }
  }
}

/**
 * @brief Main loop (unused - real loops are in FreeRTOS tasks)
 */
void loop() {}

/*********************************************************************************************************************/
/*                                           Initialization Functions                                               */
/*********************************************************************************************************************/

/**
 * @brief Initialize OLED display and rotary encoder
 * @details Performed separately from HW init to reduce startup time
 */
void initDisplayAndEncoder() 
{
  uint16_t rc;
  /***** OLED Display Setup *****/
  rc = obdI2CInit(&g_obd, MY_OLED, OLED_ADDR, FLIP180, INVERT_DISP, USE_HW_I2C, SDA1_PIN, SCL1_PIN, RESET_PIN, 800000L);  // use standard I2C bus at 400Khz
  if (rc != OLED_NOT_FOUND) 
  {
    obdSetBackBuffer(&g_obd, ucBackBuffer);
    obdFill(&g_obd, OBD_WHITE, 1);
  } 
  else 
  {
    Serial.println("Error! Failed OLED Display initialization!");
  }

  /***** Encoder Setup *****/
  g_rotaryEncoder.begin();
  g_rotaryEncoder.setup(readEncoderISR);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false); /* minValue, maxValue, circleValues true|false (when max go to min and vice versa) */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);        /* Larger number = more accelearation; 0 or 1 means disabled acceleration */
}


/* Rotary Encoder ISR */
void IRAM_ATTR readEncoderISR() {
  g_rotaryEncoder.readEncoder_ISR();
}


/**
 * Initialize the stored variables with default values.
   This is done for every CAR in the CarParam array.
 */
void initStoredVariables() {
  for (int i = 0; i < CAR_MAX_COUNT; i++) {
    g_storedVar.carParam[i].minSpeed = MIN_SPEED_DEFAULT;
    g_storedVar.carParam[i].brake = BRAKE_DEFAULT;
    g_storedVar.carParam[i].maxSpeed = MAX_SPEED_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex = { THROTTLE_CURVE_INPUT_THROTTLE_DEFAULT, THROTTLE_CURVE_SPEED_DIFF_DEFAULT };
    g_storedVar.carParam[i].antiSpin = ANTISPIN_DEFAULT;
    g_storedVar.carParam[i].freqPWM = PWM_FREQ_DEFAULT;
    g_storedVar.carParam[i].carNumber = i;
    g_storedVar.carParam[i].brakeButtonReduction = BRAKE_BUTTON_REDUCTION_DEFAULT;
    g_storedVar.carParam[i].quickBrakeEnabled = QUICK_BRAKE_ENABLED_DEFAULT;
    g_storedVar.carParam[i].quickBrakeThreshold = QUICK_BRAKE_THRESHOLD_DEFAULT;
    g_storedVar.carParam[i].quickBrakeStrength = QUICK_BRAKE_STRENGTH_DEFAULT;
    sprintf(g_storedVar.carParam[i].carName, "CAR%d", i);
  }
  g_storedVar.selectedCarNumber = 0;
  g_storedVar.minTrigger_raw = 0;
#if defined(TLE493D_MAG)
  g_storedVar.maxTrigger_raw = 3600;              /* TLE493D returns angle * 10 (0-3600) */
#else
  g_storedVar.maxTrigger_raw = ACD_RESOLUTION_STEPS; /* ADC / other sensors (0-4095) */
#endif
  g_storedVar.viewMode = VIEW_MODE_LIST;
  g_storedVar.screensaverTimeout = SCREENSAVER_TIMEOUT_DEFAULT;
  g_storedVar.powerSaveTimeout   = POWER_SAVE_TIMEOUT_DEFAULT;
  g_storedVar.deepSleepTimeout   = DEEP_SLEEP_TIMEOUT_DEFAULT;
  g_storedVar.soundBoot = SOUND_BOOT_DEFAULT;
  g_storedVar.soundRace = SOUND_RACE_DEFAULT;
  g_storedVar.gridCarSelectEnabled = 0;  /* Car select in grid view disabled by default */
  g_storedVar.raceViewMode = RACE_VIEW_DEFAULT;  /* Default race view mode */
  g_storedVar.language = LANG_DEFAULT;  /* Default language */
  g_storedVar.textCase = TEXT_CASE_DEFAULT;  /* Default text case style */
  g_storedVar.listFontSize = FONT_SIZE_DEFAULT;  /* Default list view font size */
  g_storedVar.startupDelay = STARTUP_DELAY_DEFAULT;  /* Default startup delay (0 = immediate) */
  strncpy(g_storedVar.screensaverLine1, SCREENSAVER_LINE1, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine1[SCREENSAVER_TEXT_MAX - 1] = '\0';
  strncpy(g_storedVar.screensaverLine2, SCREENSAVER_LINE2, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine2[SCREENSAVER_TEXT_MAX - 1] = '\0';
  g_storedVar.statusSlot[0] = STATUS_SLOT0_DEFAULT;
  g_storedVar.statusSlot[1] = STATUS_SLOT1_DEFAULT;
  g_storedVar.statusSlot[2] = STATUS_SLOT2_DEFAULT;
  g_storedVar.statusSlot[3] = STATUS_SLOT3_DEFAULT;
}


/**
 * Initialize the menu items with the value from the stored variables.
   Insert the items one by one in the main menu global instance.
   Then insert the items (cars) in the car menu.
   To add an item:
    - update the value of MENU_ITEMS_COUNT to match the amount of items in the menu.
    - update MAX_ITEMS so that it is greater or equal MENU_ITEMS_COUNT.
    - Add the item here, in the desired position (check the comment in slot_ESC.h MenuItem_type to know more).
 */
void initMenuItems() {
  int i = 0;
  uint8_t lang = g_storedVar.language;
  /* Init menu items - using language-specific names */

  sprintf(g_mainMenu.item[i].name, "%s", getMenuName(lang, 0));  /* BRAKE */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].brake;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "%%");
  g_mainMenu.item[i].maxValue = BRAKE_MAX_VALUE;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 1));  /* SENSI */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].minSpeed;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "%%");
  g_mainMenu.item[i].maxValue = min(MIN_SPEED_MAX_VALUE, (int)g_storedVar.carParam[g_carSel].maxSpeed);
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 2));  /* ANTIS */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].antiSpin;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "ms");
  g_mainMenu.item[i].maxValue = ANTISPIN_MAX_VALUE;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 3));  /* CURVE */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "%%");
  g_mainMenu.item[i].maxValue = THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE;
  g_mainMenu.item[i].minValue = THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE;
  g_mainMenu.item[i].callback = &showCurveSelection;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 4));  /* PWM_F */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].freqPWM;
  g_mainMenu.item[i].type = VALUE_TYPE_DECIMAL;
  sprintf(g_mainMenu.item[i].unit, "k");
  g_mainMenu.item[i].maxValue = FREQ_MAX_VALUE / 100;
  g_mainMenu.item[i].minValue = FREQ_MIN_VALUE / 100;
  g_mainMenu.item[i].decimalPoint = 1;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 5));  /* B_BTN */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].brakeButtonReduction;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "%%");
  g_mainMenu.item[i].maxValue = 100;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 6));  /* QB - Quick Brake submenu */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].quickBrakeEnabled;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "");
  g_mainMenu.item[i].maxValue = 1;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = &showQuickBrakeMenu;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 7));  /* LIMIT */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].maxSpeed;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "%%");
  g_mainMenu.item[i].maxValue = MAX_SPEED_DEFAULT;
  g_mainMenu.item[i].minValue = max(5, (int)g_storedVar.carParam[g_carSel].minSpeed + 5);
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 8));  /* SETTINGS */
  g_mainMenu.item[i].value = ITEM_NO_VALUE;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "");
  g_mainMenu.item[i].maxValue = 0;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = &showSettingsMenu;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 9));  /* STATS */
  g_mainMenu.item[i].value = ITEM_NO_VALUE;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_mainMenu.item[i].unit, "");
  g_mainMenu.item[i].maxValue = 0;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = &showLapStats;

  sprintf(g_mainMenu.item[++i].name, "%s", getMenuName(lang, 10));  /* CAR or BIL */
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].carName;
  g_mainMenu.item[i].type = VALUE_TYPE_STRING;
  g_mainMenu.item[i].maxValue = CAR_MAX_COUNT - 1;  // so menu will scroll in the array (CAR_MAX_COUNT long)
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = &showSelectRenameCar;

  /* Init Car selection menu items */
  for (uint8_t j = 0; j < CAR_MAX_COUNT; j++)
  {
    sprintf(g_carMenu.item[j].name, (const char *)g_storedVar.carParam[j].carName);
    g_carMenu.item[j].value = (void *)&g_storedVar.carParam[j].carNumber;
  }
}


/**
 * Initialize the settings submenu items
 */
void initSettingsMenuItems() {
  int i = 0;
  uint8_t lang = g_storedVar.language;

  sprintf(g_settingsMenu.item[i].name, "%s", getSettingsMenuName(lang, 0));  /* POWER - opens submenu */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 1));  /* DISPLAY - opens submenu */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 2));  /* SOUND - opens submenu */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 3));  /* WIFI */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 4));  /* USB */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 5));  /* RESET */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 6));  /* TEST */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 7));  /* ABOUT/INFO */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getSettingsMenuName(lang, 8));  /* BACK */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;
}

void initDisplayMenuItems() {
  int i = 0;
  uint8_t lang = g_storedVar.language;

  sprintf(g_settingsMenu.item[i].name, "%s", getDisplayMenuName(lang, 0));  /* VIEW */
  g_settingsMenu.item[i].value = (void *)&g_storedVar.raceViewMode;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = RACE_VIEW_SIMPLE;
  g_settingsMenu.item[i].minValue = RACE_VIEW_OFF;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getDisplayMenuName(lang, 1));  /* LANG */
  g_settingsMenu.item[i].value = (void *)&g_storedVar.language;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = LANG_ACD;
  g_settingsMenu.item[i].minValue = LANG_NOR;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getDisplayMenuName(lang, 2));  /* CASE */
  g_settingsMenu.item[i].value = (void *)&g_storedVar.textCase;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = TEXT_CASE_PASCAL;
  g_settingsMenu.item[i].minValue = TEXT_CASE_UPPER;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getDisplayMenuName(lang, 3));  /* FSIZE */
  g_settingsMenu.item[i].value = (void *)&g_storedVar.listFontSize;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = FONT_SIZE_SMALL;
  g_settingsMenu.item[i].minValue = FONT_SIZE_LARGE;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getDisplayMenuName(lang, 4));  /* STATUS - opens submenu */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_INTEGER;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_settingsMenu.item[++i].name, "%s", getDisplayMenuName(lang, 5));  /* BACK */
  g_settingsMenu.item[i].value = ITEM_NO_VALUE;
  g_settingsMenu.item[i].type = VALUE_TYPE_STRING;
  sprintf(g_settingsMenu.item[i].unit, "");
  g_settingsMenu.item[i].maxValue = 0;
  g_settingsMenu.item[i].minValue = 0;
  g_settingsMenu.item[i].callback = ITEM_NO_CALLBACK;
}


/**
 * Show the Welcome screen
 */
void showScreenWelcome()
{
  /* Display "ESPEED" in large font */
  obdWriteString(&g_obd, 0, 24, 14, (char *)"ESPEED", FONT_12x16, OBD_BLACK, 1);

  /* Display version in smaller font, centered below with spacing */
  sprintf(msgStr, "v%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);
  uint8_t versionWidth = strlen(msgStr) * 6;  /* 6px per char for FONT_6x8 */
  obdWriteString(&g_obd, 0, (OLED_WIDTH - versionWidth) / 2, 36, msgStr, FONT_6x8, OBD_BLACK, 1);
}


/**
 * Show the Pre-Calibration screen, when the button is pressed during startup, but not yet released
 */
void showScreenPreCalibration() 
{
  sprintf(msgStr, "ESPEED32 v%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);  //print SW version
  obdWriteString(&g_obd, 0, 8, 0, msgStr, FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 1 * HEIGHT8x8, (char *)"Release button", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 2 * HEIGHT8x8, (char *)"to calibrate", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 3 * HEIGHT8x8, (char *)"     OR    ", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 4 * HEIGHT8x8, (char *)"remove power", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 5 * HEIGHT8x8, (char *)"  to exit  ", FONT_8x8, OBD_BLACK, 1);
}


/**
 * Show the screen indicating that the stored variables are not present in the EEPROM
 */
void showScreenNoEEPROM() 
{
  sprintf(msgStr, "ESPEED32 v%d.%d", SW_MAJOR_VERSION, SW_MINOR_VERSION);  //print SW version
  obdWriteString(&g_obd, 0, 0, 0, msgStr, FONT_8x8, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 64, 3 * HEIGHT8x8, (char *)"EEPROM NOT init!", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 5 * HEIGHT8x8, (char *)"Press button", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, (OLED_WIDTH / 2) - 48, 6 * HEIGHT8x8, (char *)"to calibrate", FONT_8x8, OBD_BLACK, 1);
}


/**
 * Show the calibration screen. It displays the current trigger raw value, as well as the max and min encountered values.
 * 
 * @param adcRaw The raw trigger value read from the ADC
 */
void showScreenCalibration(int16_t adcRaw)
{
  sprintf(msgStr, "%s", STR_CALIBRATION[g_storedVar.language]);
  int calWidth = strlen(msgStr) * 6;
  obdWriteString(&g_obd, 0, (OLED_WIDTH - calWidth) / 2, 0, msgStr, FONT_6x8, OBD_WHITE, 1);

  sprintf(msgStr, "%s", STR_PRESS_RELEASE[g_storedVar.language]);
  obdWriteString(&g_obd, 0, 0, 8, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Raw throttle %4d  ", adcRaw);
  obdWriteString(&g_obd, 0, 0, 24, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Min throttle %4d   ", g_storedVar.minTrigger_raw);
  obdWriteString(&g_obd, 0, 0, 32, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Max throttle %4d   ", g_storedVar.maxTrigger_raw);
  obdWriteString(&g_obd, 0, 0, 40, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "%s", STR_PUSH_DONE[g_storedVar.language]);
  obdWriteString(&g_obd, 0, 0, 56, msgStr, FONT_6x8, OBD_BLACK, 1);
}

/**
 * @brief Display bottom status line with throttle, car name and voltage
 * @details Common function used by both main menu and screensaver
 */
void displayStatusLine() {
  const uint8_t Y = 3 * HEIGHT12x16 + HEIGHT8x8;  /* y = 56 (bottom status row) */

  /* Fixed column x-positions for each slot (4 × 32px = 128px total).
   * Each slot renders 5 chars (30px) left-aligned in its 32px column.
   * No preliminary erase: writing 5 fixed-width chars directly over the previous 5 chars
   * eliminates the blank-flash that caused flicker. Blank slots write 5 spaces to clear. */
  static const uint8_t SLOT_X[STATUS_SLOTS] = {0, 32, 64, 96};

  /* Determine if CAR is selected in grid mode (for car name highlight) */
  bool carSelected = false;
  if (g_storedVar.viewMode == VIEW_MODE_GRID && g_storedVar.gridCarSelectEnabled) {
    if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
      carSelected = (g_encoderMainSelector == 3);
    } else {
      carSelected = (g_encoderMainSelector == 5);
    }
  }

  char buf[7];  /* 5 chars + null */
  int8_t wifiSlot = -1;

  if (isWiFiPortalActive()) {
    for (uint8_t s = 0; s < STATUS_SLOTS; s++) {
      if (g_storedVar.statusSlot[s] == STATUS_BLANK) {
        wifiSlot = (int8_t)s;
        break;
      }
    }

    /* If all slots are occupied, temporarily override slot 4 with WIFI status. */
    if (wifiSlot < 0) {
      wifiSlot = STATUS_SLOTS - 1;
    }
  }

  for (uint8_t s = 0; s < STATUS_SLOTS; s++) {
    uint16_t slot = g_storedVar.statusSlot[s];
    uint8_t color = OBD_BLACK;

    if (wifiSlot == (int8_t)s) {
      strcpy(buf, "WIFI ");
      obdWriteString(&g_obd, 0, SLOT_X[s], Y, buf, FONT_6x8, color, 1);
      continue;
    }

    switch (slot) {
      case STATUS_OUTPUT:
        if (digitalRead(BUTT_PIN) == BUTTON_PRESSED && g_escVar.trigger_norm == 0) {
          sprintf(buf, "B%3d%%", g_storedVar.carParam[g_carSel].brakeButtonReduction);
        } else {
          sprintf(buf, "%3d%%O", g_escVar.outputSpeed_pct);
        }
        color = (g_escVar.outputSpeed_pct == 100) ? OBD_WHITE : OBD_BLACK;
        break;
      case STATUS_THROTTLE: {
        uint8_t tpct = (uint8_t)((g_escVar.trigger_norm * 100UL) / THROTTLE_NORMALIZED);
        sprintf(buf, "%3d%%T", tpct);
        break;
      }
      case STATUS_CAR:
        snprintf(buf, 6, "%-5s", g_storedVar.carParam[g_carSel].carName);
        color = carSelected ? OBD_WHITE : OBD_BLACK;
        break;
      case STATUS_CURRENT: {
        uint16_t mA = g_escVar.motorCurrent_mA;
        sprintf(buf, "%2d.%01dA", mA / 1000, (mA % 1000) / 100);
        break;
      }
      case STATUS_VOLTAGE: {
        uint16_t v = g_escVar.Vin_mV;
        sprintf(buf, "%2d.%01dV", v / 1000, (v % 1000) / 100);
        break;
      }
      default:  /* STATUS_BLANK */
        strcpy(buf, "     ");
        break;
    }

    obdWriteString(&g_obd, 0, SLOT_X[s], Y, buf, FONT_6x8, color, 1);
  }
}

/**
 * @brief Display simple race mode - shows only BRAKE and SENSI with larger font
 * @details Simplified race view with just the two most important parameters
 * @param selectedItem Item selected for editing (0=BRAKE, 1=SENSI, 2=CAR if enabled)
 * @param isEditing True if currently editing a value
 */
void displayRaceModeSimple(uint8_t selectedItem, bool isEditing) {
  static uint16_t lastBrake = 999;
  static uint16_t lastSensi = 999;
  static uint8_t lastSelectedItem = 255;
  static bool lastIsEditing = false;
  static uint16_t lastCarSel = 999;

  /* Simple mode: 0=BRAKE, 1=SENSI, 2=CAR (if enabled) */
  int col1_center = 32;   /* Center of left half (128/4 = 32) */
  int col2_center = 96;   /* Center of right half (128*3/4 = 96) */

  /* Check if forced redraw, selection or car changed - force update of ALL items */
  if (g_forceRaceRedraw || selectedItem != lastSelectedItem || isEditing != lastIsEditing || g_carSel != lastCarSel) {
    lastBrake = 999;
    lastSensi = 999;
    lastSelectedItem = selectedItem;
    lastIsEditing = isEditing;
    lastCarSel = g_carSel;
  }

  /* Determine colors based on selection state */
  uint8_t colorBrake = (selectedItem == 0) ? OBD_WHITE : OBD_BLACK;
  uint8_t colorSensi = (selectedItem == 1) ? OBD_WHITE : OBD_BLACK;

  /* BRAKE - left column, using FONT_12x16 for both label and value */
  if (g_storedVar.carParam[g_carSel].brake != lastBrake) {
    /* Label - using language-specific text with FONT_12x16: 5 chars × 12px = 60px wide */
    const char* brakeLabel = getRaceLabel(g_storedVar.language, 0);
    uint8_t labelWidth = strlen(brakeLabel) * 12;
    obdWriteString(&g_obd, 0, col1_center - (labelWidth / 2), 0, (char *)brakeLabel, FONT_12x16, colorBrake, 1);
    /* Value - "100%" with FONT_12x16: 4 chars × 12px = 48px wide, center at col1_center - 24 */
    sprintf(msgStr, "%3d%%", g_storedVar.carParam[g_carSel].brake);
    obdWriteString(&g_obd, 0, col1_center - 24, 16, msgStr, FONT_12x16, colorBrake, 1);
    lastBrake = g_storedVar.carParam[g_carSel].brake;
  }

  /* SENSI - right column, using FONT_12x16 for both label and value */
  if (g_storedVar.carParam[g_carSel].minSpeed != lastSensi) {
    /* Label - using language-specific text with FONT_12x16: 5 chars × 12px = 60px wide */
    const char* sensiLabel = getRaceLabel(g_storedVar.language, 1);
    uint8_t labelWidth = strlen(sensiLabel) * 12;
    obdWriteString(&g_obd, 0, col2_center - (labelWidth / 2), 0, (char *)sensiLabel, FONT_12x16, colorSensi, 1);
    /* Value - "100%" with FONT_12x16: 4 chars × 12px = 48px wide */
    sprintf(msgStr, "%3d%%", g_storedVar.carParam[g_carSel].minSpeed);
    obdWriteString(&g_obd, 0, col2_center - 24, 16, msgStr, FONT_12x16, colorSensi, 1);
    lastSensi = g_storedVar.carParam[g_carSel].minSpeed;
  }

  /* Note: Car name, voltage, and LIMITER warning are displayed by displayStatusLine() */
}


/**
 * @brief Display race mode - compact grid showing all key parameters
 * @details Shows throttle, brake, sensitivity, antispin, curve, limit, car name and voltage
 * @param selectedItem Item selected for editing (0-5, or 255 if none selected)
 * @param isEditing True if currently editing a value
 */
void displayRaceMode(uint8_t selectedItem, bool isEditing) {
  static uint16_t lastBrake = 999;
  static uint16_t lastSensi = 999;
  static uint16_t lastAntis = 999;
  static uint16_t lastCurve = 999;
  static uint8_t lastSelectedItem = 255;
  static bool lastIsEditing = false;
  static uint16_t lastCarSel = 999;

  /* Vertical layout - label above value, centered */
  /* Grid items: 0=BRAKE, 1=SENSI, 2=ANTIS, 3=CURVE, 4=CAR (if enabled) */
  int col1_center = 32;   /* Center of left half (128/4 = 32) */
  int col2_center = 96;   /* Center of right half (128*3/4 = 96) */

  /* Check if forced redraw, selection or car changed - force update of ALL items */
  if (g_forceRaceRedraw || selectedItem != lastSelectedItem || isEditing != lastIsEditing || g_carSel != lastCarSel) {
    /* Force redraw of all items when selection changes */
    lastBrake = 999;
    lastSensi = 999;
    lastAntis = 999;
    lastCurve = 999;

    lastSelectedItem = selectedItem;
    lastIsEditing = isEditing;
    lastCarSel = g_carSel;
  }

  /* Determine colors based on selection state */
  uint8_t colorBrake = (selectedItem == 0) ? OBD_WHITE : OBD_BLACK;
  uint8_t colorSensi = (selectedItem == 1) ? OBD_WHITE : OBD_BLACK;
  uint8_t colorAntis = (selectedItem == 2) ? OBD_WHITE : OBD_BLACK;
  uint8_t colorCurve = (selectedItem == 3) ? OBD_WHITE : OBD_BLACK;
  uint8_t colorCar = (selectedItem == 4) ? OBD_WHITE : OBD_BLACK;

  /* BRAKE - left column */
  if (g_storedVar.carParam[g_carSel].brake != lastBrake) {
    /* Label - using language-specific text, dynamically centered */
    const char* brakeLabel = getRaceLabel(g_storedVar.language, 0);
    uint8_t labelWidth = strlen(brakeLabel) * 6;
    obdWriteString(&g_obd, 0, col1_center - (labelWidth / 2), 2, (char *)brakeLabel, FONT_6x8, colorBrake, 1);
    /* Value - "100%" is 4 chars × 8px = 32px wide, center at col1_center - 16 */
    sprintf(msgStr, "%3d%%", g_storedVar.carParam[g_carSel].brake);
    obdWriteString(&g_obd, 0, col1_center - 16, 12, msgStr, FONT_8x8, colorBrake, 1);
    lastBrake = g_storedVar.carParam[g_carSel].brake;
  }

  /* SENSI - right column */
  if (g_storedVar.carParam[g_carSel].minSpeed != lastSensi) {
    /* Label - using language-specific text, shifted 1px right */
    const char* sensiLabel = getRaceLabel(g_storedVar.language, 1);
    uint8_t labelWidth = strlen(sensiLabel) * 6;
    obdWriteString(&g_obd, 0, col2_center - (labelWidth / 2) + 1, 2, (char *)sensiLabel, FONT_6x8, colorSensi, 1);
    /* Value */
    sprintf(msgStr, "%3d%%", g_storedVar.carParam[g_carSel].minSpeed);
    obdWriteString(&g_obd, 0, col2_center - 16, 12, msgStr, FONT_8x8, colorSensi, 1);
    lastSensi = g_storedVar.carParam[g_carSel].minSpeed;
  }

  /* ANTIS - left column, lower */
  if (g_storedVar.carParam[g_carSel].antiSpin != lastAntis) {
    /* Label - using language-specific text, dynamically centered */
    const char* antisLabel = getRaceLabel(g_storedVar.language, 2);
    uint8_t labelWidth = strlen(antisLabel) * 6;
    obdWriteString(&g_obd, 0, col1_center - (labelWidth / 2), 24, (char *)antisLabel, FONT_6x8, colorAntis, 1);
    /* Value - "255ms" is 5 chars × 8px = 40px wide, center at col1_center - 20 */
    sprintf(msgStr, "%3dms", g_storedVar.carParam[g_carSel].antiSpin);
    obdWriteString(&g_obd, 0, col1_center - 20, 34, msgStr, FONT_8x8, colorAntis, 1);
    lastAntis = g_storedVar.carParam[g_carSel].antiSpin;
  }

  /* CURVE - right column, lower */
  if (g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff != lastCurve) {
    /* Label - using language-specific text, shifted 1px right */
    const char* curveLabel = getRaceLabel(g_storedVar.language, 3);
    uint8_t labelWidth = strlen(curveLabel) * 6;
    obdWriteString(&g_obd, 0, col2_center - (labelWidth / 2) + 1, 24, (char *)curveLabel, FONT_6x8, colorCurve, 1);
    /* Value */
    sprintf(msgStr, "%3d%%", g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff);
    obdWriteString(&g_obd, 0, col2_center - 16, 34, msgStr, FONT_8x8, colorCurve, 1);
    lastCurve = g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
  }

  /* Note: Car name, voltage, and LIMITER warning are displayed by displayStatusLine() */
  /* CAR selection (item 4) uses the existing car name display in status line when enabled */
}

/**
 * @brief Show screensaver with branding
 * @details Displays personalized branding and optional LIMITER warning (full screen, no status line).
 *          Text is configured in screensaver_config.h (git-ignored)
 */
void showScreensaver() {
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Calculate text width for centering (16 pixels per character for FONT_16x32) */
  int line1_width = strlen(g_storedVar.screensaverLine1) * 16;
  int line1_x = (OLED_WIDTH - line1_width) / 2;

  /* Display main text in extra large font centered */
  obdWriteString(&g_obd, 0, line1_x, 8, g_storedVar.screensaverLine1, FONT_16x32, OBD_BLACK, 1);

  /* Calculate text width for centering (6 pixels per character for FONT_6x8) */
  int line2_width = strlen(g_storedVar.screensaverLine2) * 6;
  int line2_x = (OLED_WIDTH - line2_width) / 2;

  /* Display subtitle in smaller font centered below */
  obdWriteString(&g_obd, 0, line2_x, 34, g_storedVar.screensaverLine2, FONT_6x8, OBD_BLACK, 1);

  /* Display LIMITER warning if active at same position as in menu */
  if (g_storedVar.carParam[g_carSel].maxSpeed < MAX_SPEED_DEFAULT)
  {
    obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)STR_LIMITER[g_storedVar.language], FONT_8x8, OBD_WHITE, 1);
  }
}

/**
 * Print the main menu.
 * Takes care of printing the menu items and to scroll them according to the encoder position.
 * The current menu state is used to highlight decide whether to highlight only the item or also the value.
 * 
 * @param currMenuState The current menu state (ITEM_SELECTION or VALUE_SELECTION)
 */
void printMainMenu(MenuState_enum currMenuState) 
{
  static uint16_t tmp = 0;
  static bool screensaverActive = false;

  /* "Frame" indicates which items are currently displayed.
     It consist of a lower and upper bound: only the items within this boundaries are displayed.
     The difference between upper and lower bound is fixed to be visibleLines
     It's important that the encoder boundaries matches the menu items (e.g., 8 items, encoder boundaries must be [1,8]) */
  static uint16_t frameUpper = 1;
  static uint16_t frameLower = 3;
  static uint8_t lastVisibleLines = 3;
  uint8_t visibleLines = getMenuLines();

  /* If font size changed, reset frame */
  if (visibleLines != lastVisibleLines) {
    frameUpper = 1;
    frameLower = visibleLines;
    lastVisibleLines = visibleLines;
    obdFill(&g_obd, OBD_WHITE, 1);
  }

  /* In encoder move out of frame, adjust frame */
  if (g_encoderMainSelector > frameLower)
  {
    frameLower = g_encoderMainSelector;
    frameUpper = frameLower - visibleLines + 1;
    obdFill(&g_obd, OBD_WHITE, 1);
    screensaverActive = false;
  }
  else if (g_encoderMainSelector < frameUpper)
  {
    frameUpper = g_encoderMainSelector;
    frameLower = frameUpper + visibleLines - 1;
    obdFill(&g_obd, OBD_WHITE, 1);
    screensaverActive = false;
  }

  /* Check what to display based on view mode */
  static uint16_t lastViewMode = 255;  /* Track view mode changes */

  if (g_storedVar.viewMode == VIEW_MODE_GRID)
  {
    /* GRID mode: always show race mode with editing capability */
    /* Grid items depend on race view mode:
       FULL: 0=BRAKE, 1=SENSI, 2=ANTIS, 3=CURVE, 4=CAR (if enabled)
       SIMPLE: 0=BRAKE, 1=SENSI, 2=CAR (if enabled) */
    static uint8_t gridSelectedItem = 0;  /* Currently selected grid item */
    static MenuState_enum gridMenuState = ITEM_SELECTION;  /* Grid edit state */

    /* Force redraw when entering GRID mode */
    if (lastViewMode != VIEW_MODE_GRID) {
      lastViewMode = VIEW_MODE_GRID;
      g_forceRaceRedraw = true;
      obdFill(&g_obd, OBD_WHITE, 1);
    }

    /* Clear screen if coming from screensaver */
    if (screensaverActive) {
      obdFill(&g_obd, OBD_WHITE, 1);
      screensaverActive = false;
    }

    /* Calculate max grid item based on race view mode */
    uint8_t maxGridItem;
    if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
      /* SIMPLE: BRAKE, SENSI, CAR (if enabled) */
      maxGridItem = g_storedVar.gridCarSelectEnabled ? 3 : 2;
    }
    else {
      /* FULL: BRAKE, SENSI, ANTIS, CURVE, CAR (if enabled) */
      maxGridItem = g_storedVar.gridCarSelectEnabled ? 5 : 4;
    }

    if (g_encoderMainSelector < 1) g_encoderMainSelector = 1;
    if (g_encoderMainSelector > maxGridItem) g_encoderMainSelector = maxGridItem;
    gridSelectedItem = g_encoderMainSelector - 1;

    /* Determine if we're editing based on menu state */
    bool isEditing = (currMenuState == VALUE_SELECTION);

    /* Call appropriate display function based on race view mode */
    if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
      displayRaceModeSimple(gridSelectedItem, isEditing);
    }
    else {
      displayRaceMode(gridSelectedItem, isEditing);
    }
    g_forceRaceRedraw = false;  /* Reset force redraw flag after display */

    /* Print LIMITER warning if LIMIT is any value other than 100% */
    if (g_storedVar.carParam[g_carSel].maxSpeed < MAX_SPEED_DEFAULT)
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)STR_LIMITER[g_storedVar.language], FONT_8x8, OBD_WHITE, 1);
    }
    else
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)"             ", FONT_8x8, OBD_BLACK, 1);
    }
  }
  else {
  /* LIST mode: show screensaver or menu */
  lastViewMode = VIEW_MODE_LIST;  /* Track that we're in LIST mode */

  /* Calculate throttle percentage for screensaver logic */
  uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

  /* Track encoder position for screensaver wake-up */
  static uint16_t screensaverEncoderPos = 0;

  /* Check for any wake-up input (throttle, encoder change, or button press) */
  if (screensaverActive) {
    uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
    if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
        currentEncoderPos != screensaverEncoderPos ||
        digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      /* Wake from screensaver */
      obdFill(&g_obd, OBD_WHITE, 1);
      screensaverActive = false;
      g_lastEncoderInteraction = millis();
    }
  }

  /* Keep resetting timeout while throttle is above threshold (prevents screensaver activation) */
  if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD) {
    g_lastEncoderInteraction = millis();
  }

  if (g_storedVar.screensaverTimeout > 0 && millis() - g_lastEncoderInteraction > (g_storedVar.screensaverTimeout * 1000UL))
  {
    /* Timeout reached and throttle below threshold - show screensaver */
    if (!screensaverActive) {
      screensaverActive = true;
      screensaverEncoderPos = g_rotaryEncoder.readEncoder();  /* Save position when entering screensaver */
      showScreensaver();
    }
    /* Auto power save: triggers after screensaverTimeout + powerSaveTimeout min of inactivity.
     * Uses g_lastEncoderInteraction so encoder noise cannot reset the timer. */
    if (g_storedVar.powerSaveTimeout > 0 &&
        millis() - g_lastEncoderInteraction > (g_storedVar.screensaverTimeout * 1000UL) + ((uint32_t)g_storedVar.powerSaveTimeout * 60000UL)) {
      showPowerSave(g_lastEncoderInteraction);
      g_lastEncoderInteraction = millis();  /* Restart timers after waking from soft sleep */
      screensaverActive = false;
    }
    /* Auto deep sleep: triggers after screensaverTimeout + deepSleepTimeout of inactivity. */
    if (g_storedVar.deepSleepTimeout > 0 &&
        millis() - g_lastEncoderInteraction > (g_storedVar.screensaverTimeout * 1000UL) + ((uint32_t)g_storedVar.deepSleepTimeout * 60000UL)) {
      showDeepSleep();  /* Never returns */
    }
    /* Screensaver is active - don't draw menu.
     * Yield to FreeRTOS so the IDLE task can feed the watchdog timer. */
    vTaskDelay(10);
  }
  else if (!screensaverActive)
  {
    /* Not in screensaver - show normal menu */
    /* Determine font size and dimensions based on setting */
    uint8_t menuFont = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? FONT_8x8 : FONT_12x16;
    uint8_t charWidth = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? WIDTH8x8 : WIDTH12x16;
    uint8_t lineHeight = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? 8 : HEIGHT12x16;

    for (uint8_t i = 0; i < visibleLines; i++)
    {
      /* Print item name */
      /* Item color: WHITE if item is selected, black otherwise */
      obdWriteString(&g_obd, 0, 0, i * lineHeight, g_mainMenu.item[frameUpper - 1 + i].name, menuFont, (g_encoderMainSelector - frameUpper == i) ? OBD_WHITE : OBD_BLACK, 1);

      /* Only print value if value != ITEM_NO_VALUE */
      /* Value color: WHITE if corresponding item is selected AND menu state is VALUE_SELECTION, black otherwise */
      if (g_mainMenu.item[frameUpper - 1 + i].value != ITEM_NO_VALUE) 
      {
        /* if the value is a number, cast to *(unit16_t *), then print number and unit */
        if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_INTEGER)
        {
          /* Special handling for QB item: display ON/OFF instead of 0/1 */
          if (strcmp(g_mainMenu.item[frameUpper - 1 + i].name, getMenuName(g_storedVar.language, 6)) == 0) {
            uint16_t enabled = *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value);
            sprintf(msgStr, "%3s", getOnOffLabel(g_storedVar.language, enabled ? 1 : 0));
          } else {
            /* value is a generic pointer to void, so first cast to uint16_t pointer, then take the pointed value */
            sprintf(msgStr, "%3d%s", *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value), g_mainMenu.item[frameUpper - 1 + i].unit);
          }
          /* Right-align: calculate text width and position from right edge */
          int textWidth = strlen(msgStr) * charWidth;
          obdWriteString(&g_obd, 0, OLED_WIDTH - textWidth, i * lineHeight, msgStr, menuFont, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
        /* If the value is a decimal, cast to *(unit16_t *), divide by 10^decimalPoint then print number and unit */
        else if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_DECIMAL)
        {
          /* value is a generic pointer to void, so first cast to uint16_t pointer, then take the pointed value */
          tmp = *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value);
          sprintf(msgStr, " %d.%01d%s", tmp / 10, (tmp % 10), g_mainMenu.item[frameUpper - 1 + i].unit);
          /* Right-align: calculate text width and position from right edge */
          int textWidth = strlen(msgStr) * charWidth;
          obdWriteString(&g_obd, 0, OLED_WIDTH - textWidth, i * lineHeight, msgStr, menuFont, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
        /* If the value is a string, cast to (char *) then print the string */
        else if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_STRING)
        {
          /* Special handling for VIEW menu item - display language-specific labels */
          if (strcmp(g_mainMenu.item[frameUpper - 1 + i].name, "VIEW") == 0) {
            uint16_t raceViewMode = *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value);
            uint16_t lang = g_storedVar.language;
            sprintf(msgStr, "%6s", VIEW_MODE_LABELS[lang][raceViewMode]);
          }
          /* Special handling for LANG menu item */
          else if (strcmp(g_mainMenu.item[frameUpper - 1 + i].name, "LANG") == 0) {
            uint16_t language = *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value);
            const char* langLabel = (language <= LANG_ACD) ? LANG_LABELS[language] : LANG_LABELS[LANG_ENG];
            sprintf(msgStr, "%3s", langLabel);
          }
          else {
            /* value is a generic pointer to void, so cast to string pointer */
            sprintf(msgStr, "%s", (char *)(g_mainMenu.item[frameUpper - 1 + i].value));
          }
          /* Right-align: calculate text width and position from right edge */
          int textWidth = strlen(msgStr) * charWidth;
          obdWriteString(&g_obd, 0, OLED_WIDTH - textWidth, i * lineHeight, msgStr, menuFont, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
      }
    }

    initMenuItems();  /* update menu items with the storedVar that could have been changed in previous for cycle */
    /* Print LIMITER warning if LIMIT is any value other than 100% */
    if (g_storedVar.carParam[g_carSel].maxSpeed < MAX_SPEED_DEFAULT) 
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)STR_LIMITER[g_storedVar.language], FONT_8x8, OBD_WHITE, 1);
    } 
    else
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)"             ", FONT_8x8, OBD_BLACK, 1);
    }
  }
  } /* End of else (LIST mode) */

  /* Display bottom status line – only when screensaver is not active */
  if (!screensaverActive) {
    displayStatusLine();
  }
}


#define ANTIS_SPEED_START_MIN 30  /* [%] Start anti-spin only for throttle requests above this threshold */
#define ANTIS_SPEED_START_MAX 65  /* [%] Maximum anti-spin start threshold */

/**
 * @brief Apply anti-spin control to prevent car drift
 * @details Applies a ramp to output speed to prevent sudden speed changes that cause wheel spin.
 *          Anti-spin time is the time taken from minSpeed to maxSpeed, selected by user.
 *          Anti-spin is bypassed at low duty cycles (< antispinPercStart) where current is too low to cause spinning.
 *          The antispinPercStart threshold varies with the antiSpin setting:
 *          - High antiSpin (200ms) = Low threshold (powerful motor/slippery track)
 *          - Low antiSpin = High threshold (good traction)
 * 
 * @param requestedSpeed [%] Requested output speed (0-100%)
 * @return [%] Actual output speed respecting anti-spin ramp limits
 */
uint16_t throttleAntiSpin3(uint16_t requestedSpeed) {
  static uint32_t lastOutputSpeedx1000 = 0;
  static unsigned long prevCall_uS = 0;
  
  unsigned long currCall_uS = micros();
  unsigned long deltaTime_uS = currCall_uS - prevCall_uS;
  prevCall_uS = currCall_uS;

  /* Calculate dynamic anti-spin start threshold based on anti-spin setting */
  uint16_t antispinPercStart = map((long)g_storedVar.carParam[g_carSel].antiSpin, 0, (long)ANTISPIN_MAX_VALUE, 
                                    (long)ANTIS_SPEED_START_MAX, (long)ANTIS_SPEED_START_MIN);

  /* Bypass anti-spin if disabled */
  if (g_storedVar.carParam[g_carSel].antiSpin == 0) {
    lastOutputSpeedx1000 = g_storedVar.carParam[g_carSel].minSpeed * 1000;
    return requestedSpeed;
  }

  /* Bypass anti-spin at low speeds (insufficient current for wheel spin) */
  if (requestedSpeed < antispinPercStart) {
    lastOutputSpeedx1000 = requestedSpeed * 1000;
    return requestedSpeed;
  }

  /* Allow immediate deceleration (braking) */
  if ((uint32_t)requestedSpeed * 1000 <= lastOutputSpeedx1000) {
    lastOutputSpeedx1000 = requestedSpeed * 1000;
    return requestedSpeed;
  }

  /* Apply anti-spin ramp for acceleration */
  uint16_t minSpeedTmp = max((uint16_t)g_storedVar.carParam[g_carSel].minSpeed, antispinPercStart);
  
  /* Calculate maximum allowed speed change: deltaSpeed = ((minSpeed-maxSpeed) * deltaTime) / antiSpin */
  uint32_t maxDeltaSpeedx1000 = ((g_storedVar.carParam[g_carSel].maxSpeed - minSpeedTmp) * deltaTime_uS) / 
                                 g_storedVar.carParam[g_carSel].antiSpin;

  uint32_t outputSpeedX1000;
  
  /* Check if we can increase speed by full delta or if we're close to target */
  if (lastOutputSpeedx1000 < ((uint32_t)requestedSpeed * 1000 - maxDeltaSpeedx1000)) {
    outputSpeedX1000 = lastOutputSpeedx1000 + maxDeltaSpeedx1000;
  } else {
    outputSpeedX1000 = requestedSpeed * 1000;  /* Target reached */
  }

  /* Ensure ramp starts from minSpeed, not zero */
  if (outputSpeedX1000 < g_storedVar.carParam[g_carSel].minSpeed * 1000) {
    outputSpeedX1000 = g_storedVar.carParam[g_carSel].minSpeed * 1000;
  }

  lastOutputSpeedx1000 = outputSpeedX1000;
  return outputSpeedX1000 / 1000;
}


/**
 * Accounts for a deadband in an input value.
 * 
 * @param inputVal The input value
 * @param minVal The lower bound of the input value range
 * @param maxVal The upper bound of the input value range
 * @param deadBand The deadband as absolute value (not a percentage)
 * @return If the input value is less than the minVal + deadBand, it returns minVal.
           If the input value is more than the maxVal - deadBand, it returns the maxVal.
           Otherwise, it returns the inputVal, scaled in order to range from minVal to maxVal.
 */
uint16_t addDeadBand(uint16_t inputVal, uint16_t minVal, uint16_t maxVal, uint16_t deadBand) 
{
  uint16_t retVal = 0;

  /* If the inputVal is less than deadBand_pct, return 0 */
  if (inputVal < minVal + deadBand) 
  {
    retVal = 0;
  } 
  /* If the inputVal is more than maxValue - deadBand, return maxValue */
  else if (inputVal > maxVal - deadBand) 
  {
    retVal = maxVal;
  } 
  else 
  {
    /* retVal = (THROTTLE_NORMALIZED * (inputVal - deadBand)) / (THROTTLE_NORMALIZED - 2 * deadBand); TODO: verify what this is suppose to do */
    retVal = map(inputVal, deadBand, maxVal - deadBand, minVal, maxVal);  /* Scale the inputValue (which ranges from (minVal + deadBand) to (maxVal - deadBand))
                                                                             so that it ranges from minVal to maxVal */
  }

  return retVal;
}

/**
 * throttleCurve2: Map trigger position(throttle) to speed (duty) on a broken line curve, with midpoint set as throttleCurveVertex
 * dual throttle curve: When decelerating , if dragB is higher than 100%-minSpeed, then set a lower minSpeed
 * @param inputThrottleNorm The input Trigger value, normalized between 0 and THROTTLE_NORMALIZED
 * @return duty cyle to be applied at that specific thrigger position on the selected curve
 */
uint16_t throttleCurve2(uint16_t inputThrottleNorm )
{
  uint16_t outputSpeed = 0;           /* The requested output speed (duty cycle) from 0% to 100% */
  uint32_t throttleCurveVertexSpeed;  /* The output speed when the throttle is at 50% (that is, the value of throttleCurveVertex.inputThrottle) */
  uint16_t tmpMinSpeed;               /* Minimum speed may change when decelerating due to drag brake, so create a temporary min speed */

  /* dual throttle curve: When decelerating , if dragB is higher than 100%-minSpeed, then set a lower minSpeed on the curve to allow the desired drag brake to be applied*/
  tmpMinSpeed = g_storedVar.carParam[g_carSel].minSpeed;

  /* Calculate the output speed of the throttle curve vertex
     This is calculated as the curveSpeedDiff (from 10% to 90%) percentage of the difference between minSpeed and maxSpeed */
  throttleCurveVertexSpeed = tmpMinSpeed + (((uint32_t)g_storedVar.carParam[g_carSel].maxSpeed - (uint32_t)tmpMinSpeed) * ((uint32_t)g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff) / 100);

  if (inputThrottleNorm == 0)   /* If input throttle is 0 --> output speed is 0% */
  {
    outputSpeed = 0;
  } 
  else if (inputThrottleNorm <= g_storedVar.carParam[g_carSel].throttleCurveVertex.inputThrottle) /* If the input throttle is less than the vertex point (fixed at 50%), than map the output speed from 0 to the throttleCurveVertexSpeed */
  {
    outputSpeed = map(inputThrottleNorm, 0, g_storedVar.carParam[g_carSel].throttleCurveVertex.inputThrottle, tmpMinSpeed, throttleCurveVertexSpeed);
  } 
  else  /* If the input throttle is more than the vertex point (fixed at 50%), than map the output speed from throttleCurveVertexSpeed to the maxSpeed*/
  {
    outputSpeed = map(inputThrottleNorm, g_storedVar.carParam[g_carSel].throttleCurveVertex.inputThrottle, THROTTLE_NORMALIZED, throttleCurveVertexSpeed, g_storedVar.carParam[g_carSel].maxSpeed);
  }

  return outputSpeed;
}

/**
 * Call this when calibrating the throttle.
 * Check if the parameter adcRaw is bigger/smaller than the stored max/min values, and updates them accordingly.
 * @param inputThrottle The raw ADC value
 */
void throttleCalibration(int16_t adcRaw)
{
  if (g_storedVar.maxTrigger_raw < adcRaw)
    {
      g_storedVar.maxTrigger_raw = adcRaw;
    }
  if (g_storedVar.minTrigger_raw > adcRaw)
    {
      g_storedVar.minTrigger_raw = adcRaw;
    }
}


/**
 * Reacts to the rotary encoder being clicked.
 * If the menu is in ITEM_SELECTION, it must select the correct item (update the g_encoderSelectedValuePtr) or invoking the item's callback --> then return the next state VALUE_SELECTION
 * If the menu is in VALUE_SELECTION, it must return to ITEM_SELECTION
 * 
 * @param currMenuState The current menu state.
 * @return The next menu state.
 */
MenuState_enum rotary_onButtonClick(MenuState_enum currMenuState)
{
  static unsigned long lastTimePressed = 0;
  static uint16_t selectedParamMaxValue = 100;
  static uint16_t selectedParamMinValue = 0;

  /* Ignore multiple presses that are too close together */
  if (millis() - lastTimePressed < BUTTON_SHORT_PRESS_DEBOUNCE_MS)
  {
    return currMenuState;
  }

  lastTimePressed = millis();

  if (currMenuState == ITEM_SELECTION) /* If the current state is ITEM_SELECTION */
  {
    /* Special handling for GRID mode - map grid item to car parameter */
    if (g_storedVar.viewMode == VIEW_MODE_GRID)
    {
      uint8_t gridItem = g_encoderMainSelector - 1;  /* 0-4 (FULL) or 0-2 (SIMPLE) */
      g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);

      /* Map grid item to parameter pointer and limits based on race view mode */
      if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
        /* SIMPLE mode: 0=BRAKE, 1=SENSI, 2=CAR */
        switch (gridItem) {
          case 0:  /* BRAKE */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].brake;
            selectedParamMaxValue = BRAKE_MAX_VALUE;
            selectedParamMinValue = 0;
            break;
          case 1:  /* SENSI (minSpeed) */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].minSpeed;
            selectedParamMaxValue = MIN_SPEED_MAX_VALUE;
            selectedParamMinValue = 0;
            break;
          case 2:  /* CAR - select different car */
            g_encoderSelectedValuePtr = &g_storedVar.selectedCarNumber;
            selectedParamMaxValue = CAR_MAX_COUNT - 1;
            selectedParamMinValue = 0;
            g_isEditingCarSelection = true;  /* Prevent g_carSel update during edit */
            break;
          default:
            return ITEM_SELECTION;
        }
      }
      else {
        /* FULL mode: 0=BRAKE, 1=SENSI, 2=ANTIS, 3=CURVE, 4=CAR */
        switch (gridItem) {
          case 0:  /* BRAKE */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].brake;
            selectedParamMaxValue = BRAKE_MAX_VALUE;
            selectedParamMinValue = 0;
            break;
          case 1:  /* SENSI (minSpeed) */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].minSpeed;
            selectedParamMaxValue = MIN_SPEED_MAX_VALUE;
            selectedParamMinValue = 0;
            break;
          case 2:  /* ANTIS (antiSpin) */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].antiSpin;
            selectedParamMaxValue = ANTISPIN_MAX_VALUE;
            selectedParamMinValue = 0;
            break;
          case 3:  /* CURVE */
            g_encoderSelectedValuePtr = &g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
            selectedParamMaxValue = THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE;
            selectedParamMinValue = THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE;
            break;
          case 4:  /* CAR - select different car */
            g_encoderSelectedValuePtr = &g_storedVar.selectedCarNumber;
            selectedParamMaxValue = CAR_MAX_COUNT - 1;
            selectedParamMinValue = 0;
            g_isEditingCarSelection = true;  /* Prevent g_carSel update during edit */
            break;
          default:
            return ITEM_SELECTION;
        }
      }

      g_rotaryEncoder.setBoundaries(selectedParamMinValue, selectedParamMaxValue, false);
      g_rotaryEncoder.reset(*g_encoderSelectedValuePtr);
      g_escVar.encoderPos = *g_encoderSelectedValuePtr;
      g_originalValueBeforeEdit = *g_encoderSelectedValuePtr;  /* Save original value for cancel */
      return VALUE_SELECTION;
    }
    /* LIST mode handling */
    /* If an item has no callback, go in menu state VALUE_SELECTION */
    else if (g_mainMenu.item[g_encoderMainSelector - 1].callback == ITEM_NO_CALLBACK)
    {
      g_rotaryEncoder.setAcceleration(SEL_ACCELERATION); /* Set higher encoder acceleration so it doesn't require too many turns to make a big value change */


      g_encoderSelectedValuePtr = (uint16_t *)g_mainMenu.item[g_encoderMainSelector - 1].value;   /* Update the g_encoderSelectedValuePtr to point to the value of the selected item.
                                                                                                     value is a generic pointer to void, so cast to uint16_t pointer */
      selectedParamMaxValue = g_mainMenu.item[g_encoderMainSelector - 1].maxValue;                /* Set Max and Min boundaries according to the selected items max and min value */
      selectedParamMinValue = g_mainMenu.item[g_encoderMainSelector - 1].minValue;
      g_rotaryEncoder.setBoundaries(selectedParamMinValue, selectedParamMaxValue, false);
      g_rotaryEncoder.reset(*g_encoderSelectedValuePtr);  /* Reset the encoder to the current value of the selected item */
      g_escVar.encoderPos = *g_encoderSelectedValuePtr;   /* Set the encoderPos global variable to the current value of the selected item */
      g_originalValueBeforeEdit = *g_encoderSelectedValuePtr;  /* Save original value for cancel */
      return VALUE_SELECTION;                             /* Return the VALUE_SELECTION state */
    }
    /* if an item has a callback, execute it, then return to ITEM SELECTION */
    else
    {
      g_mainMenu.item[g_encoderMainSelector - 1].callback();  /* Invoke the selected item's callback */
      return ITEM_SELECTION;
    }
  }
  else /* If the current state is VALUE_SELECTION */
  {
    g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);         /* Set the encoder acceleration to MENU_ACCELERATION */

    /* Set boundaries based on view mode */
    if (g_storedVar.viewMode == VIEW_MODE_GRID) {
      uint8_t gridItems;
      if (g_storedVar.raceViewMode == RACE_VIEW_SIMPLE) {
        gridItems = g_storedVar.gridCarSelectEnabled ? 3 : 2;  /* SIMPLE: BRAKE, SENSI, CAR (optional) */
      } else {
        gridItems = g_storedVar.gridCarSelectEnabled ? 5 : 4;  /* FULL: BRAKE, SENSI, ANTIS, CURVE, CAR (optional) */
      }
      g_rotaryEncoder.setBoundaries(1, gridItems, false);
    } else {
      g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);  /* Set the encoder boundaries to the menu boundaries */
    }

    g_rotaryEncoder.reset(g_encoderMainSelector);               /* Reset the encoder value to g_encoderMainSelector, so that it doesn't change the selected item */
    g_escVar.encoderPos = g_encoderMainSelector;

    g_isEditingCarSelection = false;  /* Clear flag - editing complete */
    saveEEPROM(g_storedVar);  /* Save modified values to EEPROM */
    return ITEM_SELECTION;    /* Return the ITEM_SELECTION state */
  }
}


/**
 * Show the Car selection menu.
 * Uses a "Frame" just like the main menu to display and scroll through the carMenu items.
 * Scrolling the Cars is actually changing the value of selectedCarNumber.
 */
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; return; }
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; return; }
  }

  /* Small delay to prevent double-click */
  delay(200);
  if (g_escapeToMain) return;

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
    if (checkRaceModeEscape()) { g_escapeToMain = true; return; }
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; goto exitCarMenu; }
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
    if (!g_escapeToMain) saveEEPROM(g_storedVar);
  }
  /* If SELECT option was selected, go to showCarSelection routine */
  else if (selectedOption == CAR_OPTION_SELECT)
  {
    showCarSelection();
    if (!g_escapeToMain) saveEEPROM(g_storedVar);
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
    if (!g_escapeToMain) saveEEPROM(g_storedVar);
  }
  /* If RESET option was selected, reset all car parameters with double confirmation */
  else if (selectedOption == CAR_OPTION_RESET)
  {
    uint16_t lang = g_storedVar.language;
    const char* carLabel = (lang == LANG_NOR) ? "ALLE BILER" : "ALL CARS";
    bool confirmed = resetConfirm(carLabel);
    if (confirmed) {
      doResetCar();
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
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
  g_escVar.encoderPos = g_encoderMainSelector;
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; return; }
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
  uint16_t throttleCurveVertexSpeed;
  uint16_t prevTrigger = g_escVar.outputSpeed_pct;
  uint16_t inputThrottle = (g_storedVar.carParam[g_carSel].throttleCurveVertex.inputThrottle * 100) / THROTTLE_NORMALIZED;  //Take inputThrottle (from 0 to THROTTLE NORMALIZED) and convert it in a 0% to 100% value
  
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
  obdWriteString(&g_obd, 0, 0, map(g_storedVar.carParam[g_carSel].minSpeed, 0, 100, 50, 8), (char *)"MIN", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 28, 50 - (g_storedVar.carParam[g_carSel].maxSpeed / 2), (char *)"MAX", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 64, 58, (char *)"50%", FONT_6x8, OBD_BLACK, 1);

  /* Draw axis ticks at 50%, MIN SPEED and MAX speed points*/
  obdSetPixel(&g_obd, 24, 50 - (g_storedVar.carParam[g_carSel].minSpeed / 2), OBD_BLACK, 1);
  obdSetPixel(&g_obd, 23, 50 - (g_storedVar.carParam[g_carSel].minSpeed / 2), OBD_BLACK, 1);
  obdSetPixel(&g_obd, 25 + inputThrottle, 51, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 25 + inputThrottle, 52, OBD_BLACK, 1);
  obdSetPixel(&g_obd, 26, 50 - (g_storedVar.carParam[g_carSel].maxSpeed / 2), OBD_BLACK, 1);
  obdSetPixel(&g_obd, 27, 50 - (g_storedVar.carParam[g_carSel].maxSpeed / 2), OBD_BLACK, 1);

  /* Set encoder to curve parameters */
  g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
  g_rotaryEncoder.setBoundaries(THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE, THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE, false);
  g_rotaryEncoder.reset(g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff);

  /* Save original curve value for cancel */
  uint16_t originalCurveValue = g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
  bool curveCanceled = false;

  /* Calculate the output speed of the throttle curve vertex (as in throttleCurve() function)
     This is calculated as the curveSpeedDiff (from 10% to 90%) percentage of the difference between minSpeed and maxSpeed */
  throttleCurveVertexSpeed = g_storedVar.carParam[g_carSel].minSpeed + (((uint32_t)g_storedVar.carParam[g_carSel].maxSpeed - (uint32_t)g_storedVar.carParam[g_carSel].minSpeed) * ((uint32_t)g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff) / 100);

  /* Draw Line from MIN SPEED, to middle point */
  obdDrawLine(&g_obd, 25, 50 - (g_storedVar.carParam[g_carSel].minSpeed / 2), 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), OBD_BLACK, 1);
  /* Draw Line from middle point to 100% */
  obdDrawLine(&g_obd, 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), 125, map(g_storedVar.carParam[g_carSel].maxSpeed, 0, 100, 50, 0), OBD_BLACK, 1);

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
      obdDrawLine(&g_obd, 25, 50 - (g_storedVar.carParam[g_carSel].minSpeed / 2), 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), OBD_WHITE, 1);
      obdDrawLine(&g_obd, 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), 125, map(g_storedVar.carParam[g_carSel].maxSpeed, 0, 100, 50, 0), OBD_WHITE, 1);
      
      /* Update the speed coordinate of the vertex */
      g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff = g_rotaryEncoder.readEncoder();
      sprintf(msgStr, "%3d%c", g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff, '%');
      throttleCurveVertexSpeed = g_storedVar.carParam[g_carSel].minSpeed + ((uint32_t)g_storedVar.carParam[g_carSel].maxSpeed - (uint32_t)g_storedVar.carParam[g_carSel].minSpeed) * ((uint32_t)g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff) / 100;
      obdWriteString(&g_obd, 0, OLED_WIDTH - 48, 34, msgStr, FONT_12x16, OBD_BLACK, 1);
      
      /* Draw the new lines */
      obdDrawLine(&g_obd, 25, 50 - (g_storedVar.carParam[g_carSel].minSpeed / 2), 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), OBD_BLACK, 1);
      obdDrawLine(&g_obd, 25 + inputThrottle, map(throttleCurveVertexSpeed, 0, 100, 50, 0), 125, map(g_storedVar.carParam[g_carSel].maxSpeed, 0, 100, 50, 0), OBD_BLACK, 1);
    }
    else
    {
      /* Service the watchdog, to prevent CPU reset */
      vTaskDelay(10);
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }
  }

  /* Reset encoder */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
  g_escVar.encoderPos = g_encoderMainSelector;

  /* Only save if not canceled */
  if (!curveCanceled && !g_escapeToMain) {
    saveEEPROM(g_storedVar);          /* Save modified values to EEPROM */
  }

  obdFill(&g_obd, OBD_WHITE, 1);    /* Clear screen */

  return;
}


/**
 * Show the Lap Stats screen
 * Displays lap count, best time, current lap time, motor current,
 * and a scrollable list of the last 20 lap times.
 * Encoder scrolls through lap list, button click returns to main menu.
 * Brake button resets lap counter.
 */
void showLapStats() {
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Encoder for scrolling lap list (4 visible rows) */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, 0, false);
  g_rotaryEncoder.reset(0);

  int16_t scrollPos = 0;
  uint16_t prevLapCount = 0xFFFF;
  uint32_t prevBestLap = 0xFFFF;
  uint32_t prevCurrentLap = 0xFFFF;
  uint16_t prevMotorCurrent = 0xFFFF;
  int16_t prevScrollPos = -1;
  bool needFullRedraw = true;

  /* Screensaver support */
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  /* Title - y=0 (pixel row 0), FONT_8x8 = 8px tall */
  const char* title = "Stats";
  uint8_t titleWidth = strlen(title) * 8;
  obdWriteString(&g_obd, 0, (OLED_WIDTH - titleWidth) / 2, 0, (char *)title, FONT_8x8, OBD_WHITE, 1);

  while (true) {
    /* Screensaver handling */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != screensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUp = true;
        screensaverActive = false;
        lastInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
        needFullRedraw = true;
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
          needFullRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    /* Button click = return to main menu */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }

    /* Brake button = back (exit to main menu) */
    static bool brakeInStats = false;
    static uint32_t lastBrakeStats = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeInStats && millis() - lastBrakeStats > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeInStats = true;
        lastBrakeStats = millis();
        break;
      }
    } else {
      brakeInStats = false;
    }

    /* Update encoder scroll boundaries based on lap count */
    uint16_t totalLaps = g_escVar.lapCount;
    uint16_t displayLaps = min((uint16_t)LAP_MAX_COUNT, totalLaps);
    if (displayLaps > 4) {
      g_rotaryEncoder.setBoundaries(0, displayLaps - 4, false);
    } else {
      g_rotaryEncoder.setBoundaries(0, 0, false);
    }
    scrollPos = g_rotaryEncoder.readEncoder();

    /* Redraw title if needed */
    if (needFullRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);
      obdWriteString(&g_obd, 0, (OLED_WIDTH - titleWidth) / 2, 0, (char *)title, FONT_8x8, OBD_WHITE, 1);
      prevLapCount = 0xFFFF;
      prevBestLap = 0xFFFF;
      prevCurrentLap = 0xFFFF;
      prevMotorCurrent = 0xFFFF;
      prevScrollPos = -1;
      needFullRedraw = false;
    }

    /* Row 1 (y=8): "Laps:XX  Best:X.XXs" (FONT_6x8, 21 chars max) */
    bool lapCountChanged = (totalLaps != prevLapCount);
    if (lapCountChanged || g_escVar.bestLapTime_ms != prevBestLap) {
      if (totalLaps > 0) {
        sprintf(msgStr, "Laps:%-3d Best:%d.%02ds",
                totalLaps,
                (int)(g_escVar.bestLapTime_ms / 1000),
                (int)((g_escVar.bestLapTime_ms % 1000) / 10));
      } else {
        sprintf(msgStr, "Laps:0              ");
      }
      obdWriteString(&g_obd, 0, 0, 8, msgStr, FONT_6x8, OBD_BLACK, 1);
      prevLapCount = totalLaps;
      prevBestLap = g_escVar.bestLapTime_ms;
    }

    /* Row 2 (y=16): "Curr:X.Xs  mA:XXXX" (FONT_6x8) */
    uint32_t currentLapTime = 0;
    if (g_escVar.lapStartTime_ms > 0) {
      currentLapTime = millis() - g_escVar.lapStartTime_ms;
    }
    uint16_t motorCurrent = g_escVar.motorCurrent_mA;
    /* Update every ~200ms to avoid flicker */
    if ((currentLapTime / 200) != (prevCurrentLap / 200) || motorCurrent != prevMotorCurrent) {
      if (g_escVar.lapStartTime_ms > 0) {
        sprintf(msgStr, "Curr:%d.%ds  mA:%-4d",
                (int)(currentLapTime / 1000),
                (int)((currentLapTime % 1000) / 100),
                motorCurrent);
      } else {
        sprintf(msgStr, "Curr:---   mA:%-4d ", motorCurrent);
      }
      obdWriteString(&g_obd, 0, 0, 16, msgStr, FONT_6x8, OBD_BLACK, 1);
      prevCurrentLap = currentLapTime;
      prevMotorCurrent = motorCurrent;
    }

    /* Rows 3-6 (y=24,32,40,48): Scrollable lap list, newest first */
    if (scrollPos != prevScrollPos || lapCountChanged) {
      for (uint8_t row = 0; row < 4; row++) {
        uint8_t yPixel = 24 + (row * 8);
        /* Lap index: newest first, with scroll offset */
        int16_t lapIdx = (int16_t)totalLaps - 1 - scrollPos - row;
        if (lapIdx >= 0 && lapIdx < (int16_t)totalLaps) {
          uint8_t bufIdx = lapIdx % LAP_MAX_COUNT;
          uint32_t t = g_escVar.lapTimes[bufIdx];
          bool isBest = (t == g_escVar.bestLapTime_ms && t > 0);
          sprintf(msgStr, "#%-3d %d.%02ds %s       ",
                  lapIdx + 1,
                  (int)(t / 1000),
                  (int)((t % 1000) / 10),
                  isBest ? "*" : " ");
          msgStr[21] = '\0';
          obdWriteString(&g_obd, 0, 0, yPixel, msgStr, FONT_6x8, OBD_BLACK, 1);
        } else {
          obdWriteString(&g_obd, 0, 0, yPixel, (char *)"                     ", FONT_6x8, OBD_BLACK, 1);
        }
      }
      prevScrollPos = scrollPos;
    }

    /* Row 7 (y=56): Status line (throttle, car name, voltage) */
    displayStatusLine();

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    delay(50);  /* ~20 Hz UI refresh */
  }

  /* Restore main menu encoder settings */
  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
}


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
  g_rotaryEncoder.setBoundaries(0, TEXT_COLS, false);  /* 0..20 = chars, 21 = OK */
  g_rotaryEncoder.reset(0);

  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  while (true) {
    /* Screensaver handling */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = g_rotaryEncoder.readEncoder();
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
        screensaverEncoderPos = g_rotaryEncoder.readEncoder();
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
        cursorPos = (uint8_t)g_rotaryEncoder.readEncoder();
      } else {
        tmpText[cursorPos] = (char)g_rotaryEncoder.readEncoder();
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
        g_rotaryEncoder.setBoundaries(RENAME_CAR_MIN_ASCII, RENAME_CAR_MAX_ASCII, false);
        g_rotaryEncoder.reset((uint8_t)tmpText[cursorPos]);
      } else {
        editMode = RENAME_CAR_SELECT_OPTION_MODE;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, TEXT_COLS, false);
        g_rotaryEncoder.reset(cursorPos);
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

  const char* ssNames[SS_ITEMS];
  if (lang == LANG_NOR) {
    ssNames[0] = "LINJE1";  ssNames[1] = "LINJE2";
    ssNames[2] = "TID";     ssNames[3] = "NA";
    ssNames[4] = "TILBAKE";
  } else {
    ssNames[0] = "LINE1";   ssNames[1] = "LINE2";
    ssNames[2] = "TIME";    ssNames[3] = "NOW";
    ssNames[4] = "BACK";
  }
  const char* editorTitleL1 = (lang == LANG_NOR) ? "Linje 1" : "Line 1";
  const char* editorTitleL2 = (lang == LANG_NOR) ? "Linje 2" : "Line 2";
  const char* editorTitleTime = (lang == LANG_NOR) ? "Tid" : "Time";
  (void)editorTitleTime;  /* used below in time edit label */

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, SS_ITEMS, false);
  g_rotaryEncoder.reset(1);

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
      uint16_t ep = g_rotaryEncoder.readEncoder();
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
          screensaverEncoderPos = g_rotaryEncoder.readEncoder();
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
          g_rotaryEncoder.setBoundaries(1, SS_ITEMS, false);
          g_rotaryEncoder.reset(3);  /* Return to TIME item */
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
        g_storedVar.screensaverTimeout = g_rotaryEncoder.readEncoder();
        prevSel = 0xFF;  /* Redraw TIME row value */
      } else {
        sel = (uint8_t)g_rotaryEncoder.readEncoder();
      }
    }

    /* Draw items if changed */
    if (sel != prevSel || inTimeEdit) {
      for (uint8_t idx = 0; idx < SS_ITEMS; idx++) {
        uint8_t yPx = (idx + 1) * HEIGHT8x8;  /* y=8,16,24,32,40 */
        bool isSelected = (sel == idx + 1);
        bool isEditingThis = (inTimeEdit && idx == 2);  /* TIME row */

        /* Item name */
        obdWriteString(&g_obd, 0, 0, yPx, (char *)ssNames[idx], FONT_8x8,
                       (isSelected || isEditingThis) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value on right side in FONT_6x8 */
        if (idx == 0) {
          /* LINE1: show first 10 chars, right-justified */
          snprintf(msgStr, 11, "%10s", g_storedVar.screensaverLine1);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, yPx, msgStr, FONT_6x8, OBD_BLACK, 1);
        } else if (idx == 1) {
          /* LINE2: show first 10 chars, right-justified */
          snprintf(msgStr, 11, "%10s", g_storedVar.screensaverLine2);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, yPx, msgStr, FONT_6x8, OBD_BLACK, 1);
        } else if (idx == 2) {
          /* TIME: show current timeout, right-justified (4 chars × 6px = 24px from right) */
          if (g_storedVar.screensaverTimeout == 0) {
            sprintf(msgStr, "%4s", (lang == LANG_NOR) ? "AV" : "OFF");
          } else {
            sprintf(msgStr, "%3ds", g_storedVar.screensaverTimeout);
          }
          obdWriteString(&g_obd, 0, OLED_WIDTH - 24, yPx, msgStr, FONT_6x8,
                         isEditingThis ? OBD_WHITE : OBD_BLACK, 1);
        }
        /* idx==3 (NOW) and idx==4 (BACK) have no value — name only */
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
        g_rotaryEncoder.setBoundaries(1, SS_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 1) {
        editScreensaverText(g_storedVar.screensaverLine1, editorTitleL1);
        saveEEPROM(g_storedVar);
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, SS_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 2) {
        editScreensaverText(g_storedVar.screensaverLine2, editorTitleL2);
        saveEEPROM(g_storedVar);
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, SS_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel = 0xFF;
      } else if (sel == 3) {
        /* Enter TIME edit mode */
        inTimeEdit = true;
        origTimeout = g_storedVar.screensaverTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, SCREENSAVER_TIMEOUT_MAX, false);
        g_rotaryEncoder.reset(g_storedVar.screensaverTimeout);
      } else if (sel == 4) {
        /* NOW: activate screensaver immediately */
        screensaverActive = true;
        screensaverEncoderPos = g_rotaryEncoder.readEncoder();
        showScreensaver();
        lastInteraction = millis();
      } else if (sel == 5) {
        break;  /* BACK */
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * Status line slot settings submenu.
 * Items: SLOT 1–4 (select content per fixed column) + BACK.
 * Live preview of the status bar is shown at y=56 while browsing and editing.
 * Follows the same ITEM_SELECTION / VALUE_SELECTION pattern as showSettingsMenu().
 */
void showStatusSettings() {
  /* ST_ITEMS: 4 slots + BACK */
  const uint8_t ST_ITEMS    = STATUS_SLOTS + 1;
  /* Slot content range: STATUS_BLANK..STATUS_VOLTAGE */
  const uint8_t ST_SLOT_MAX = STATUS_VOLTAGE;

  uint8_t lang = g_storedVar.language;

  /* Row labels */
  const char* rowNames[ST_ITEMS];
  if (lang == LANG_NOR) {
    rowNames[0] = "FELT 1"; rowNames[1] = "FELT 2";
    rowNames[2] = "FELT 3"; rowNames[3] = "FELT 4";
    rowNames[4] = "TILBAKE";
  } else {
    rowNames[0] = "SLOT 1"; rowNames[1] = "SLOT 2";
    rowNames[2] = "SLOT 3"; rowNames[3] = "SLOT 4";
    rowNames[4] = "BACK";
  }

  /* Content type labels (max 4 chars, shown right-justified in menu) */
  const char* slotLabels[ST_SLOT_MAX + 1];
  if (lang == LANG_NOR) {
    slotLabels[STATUS_BLANK]    = "---";
    slotLabels[STATUS_OUTPUT]   = "OUT%";
    slotLabels[STATUS_THROTTLE] = "GASS";
    slotLabels[STATUS_CAR]      = "BIL";
    slotLabels[STATUS_CURRENT]  = "AMPE";
    slotLabels[STATUS_VOLTAGE]  = "VOLT";
  } else {
    slotLabels[STATUS_BLANK]    = "---";
    slotLabels[STATUS_OUTPUT]   = "OUT%";
    slotLabels[STATUS_THROTTLE] = "THRO";
    slotLabels[STATUS_CAR]      = "CAR";
    slotLabels[STATUS_CURRENT]  = "CURR";
    slotLabels[STATUS_VOLTAGE]  = "VOLT";
  }

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, ST_ITEMS, false);
  g_rotaryEncoder.reset(1);

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
      uint16_t ep = g_rotaryEncoder.readEncoder();
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
          screensaverEncPos = g_rotaryEncoder.readEncoder();
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
          g_rotaryEncoder.setBoundaries(1, ST_ITEMS, false);
          g_rotaryEncoder.reset(sel);
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
      uint16_t ep = (uint16_t)g_rotaryEncoder.readEncoder();
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

        /* Value label right-justified (4 chars × 6px = 24px from right edge) */
        if (idx < STATUS_SLOTS) {
          uint16_t v = g_storedVar.statusSlot[idx];
          const char* lbl = (v <= ST_SLOT_MAX) ? slotLabels[v] : "???";
          char vbuf[5];
          snprintf(vbuf, sizeof(vbuf), "%4s", lbl);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 24, yPx, vbuf, FONT_6x8,
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
        g_rotaryEncoder.setBoundaries(0, ST_SLOT_MAX, false);
        g_rotaryEncoder.reset(origValue);
        state       = VALUE_SELECTION;
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      } else {
        /* Confirm: save and return to item selection */
        saveEEPROM(g_storedVar);
        state = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, ST_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * Show a double-click confirmation for a destructive reset action.
 * @param label  Short label shown on the confirm screen (e.g. "CAR")
 * @return true if user confirmed twice, false if cancelled
 */
static bool resetConfirm(const char* label) {
  uint16_t lang = g_storedVar.language;
  const char* cancelText = (lang == LANG_NOR) ? "TILBAKE=AVBRYT" : "BACK=CANCEL";

  auto showCancelled = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    const char* msg = (lang == LANG_NOR) ? "AVBRUTT!" : "CANCELLED!";
    int tw = strlen(msg) * WIDTH12x16;
    obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 24, (char *)msg, FONT_12x16, OBD_BLACK, 1);
    delay(1000);
    obdFill(&g_obd, OBD_WHITE, 1);
  };

  const char* pressT = (lang == LANG_NOR) ? "TRYKK 2 GANGER" : "PRESS TWICE";
  auto drawFirstConfirm = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    obdWriteString(&g_obd, 0, 10, 5,  (char *)label, FONT_8x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 10, 25, (char *)pressT,  FONT_6x8, OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 10, 45, (char *)cancelText, FONT_6x8, OBD_BLACK, 1);
  };

  auto drawSecondConfirm = [&]() {
    obdFill(&g_obd, OBD_WHITE, 1);
    if (lang == LANG_NOR) {
      obdWriteString(&g_obd, 0, 10, 5,  (char *)"TRYKK IGJEN",     FONT_8x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 16, (char *)"FOR A NULLSTILLE", FONT_6x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 24, (char *)label,              FONT_6x8, OBD_BLACK, 1);
    } else {
      obdWriteString(&g_obd, 0, 10, 5,  (char *)"PRESS AGAIN",      FONT_8x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 16, (char *)"TO RESET",         FONT_6x8, OBD_BLACK, 1);
      obdWriteString(&g_obd, 0, 10, 24, (char *)label,              FONT_6x8, OBD_BLACK, 1);
    }
    obdWriteString(&g_obd, 0, 10, 45, (char *)cancelText, FONT_6x8, OBD_BLACK, 1);
  };

  /* First confirmation */
  drawFirstConfirm();
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
        drawFirstConfirm();
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
          drawFirstConfirm();
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        drawFirstConfirm();
      }
      delay(10);
      continue;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      showCancelled();
      return false;
    }
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
    }
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }
    delay(10);
  }
  delay(200);

  /* Second confirmation - specific label */
  drawSecondConfirm();
  lastInteraction = millis();
  screensaverActive = false;
  screensaverEncoderPos = 0;

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
        drawSecondConfirm();
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
          drawSecondConfirm();
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        drawSecondConfirm();
      }
      delay(10);
      continue;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      showCancelled();
      return false;
    }
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
    }
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      break;
    }
    delay(10);
  }
  delay(200);
  return true;
}

/** Reset all car profiles to factory defaults. */
static void doResetCar() {
  for (uint8_t i = 0; i < CAR_MAX_COUNT; i++) {
    g_storedVar.carParam[i].minSpeed   = MIN_SPEED_DEFAULT;
    g_storedVar.carParam[i].brake      = BRAKE_DEFAULT;
    g_storedVar.carParam[i].antiSpin   = ANTISPIN_DEFAULT;
    g_storedVar.carParam[i].maxSpeed   = MAX_SPEED_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex.inputThrottle = THROTTLE_CURVE_INPUT_THROTTLE_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex.curveSpeedDiff = THROTTLE_CURVE_SPEED_DIFF_DEFAULT;
    g_storedVar.carParam[i].freqPWM              = PWM_FREQ_DEFAULT;
    g_storedVar.carParam[i].brakeButtonReduction = BRAKE_BUTTON_REDUCTION_DEFAULT;
    g_storedVar.carParam[i].quickBrakeEnabled    = QUICK_BRAKE_ENABLED_DEFAULT;
    g_storedVar.carParam[i].quickBrakeThreshold  = QUICK_BRAKE_THRESHOLD_DEFAULT;
    g_storedVar.carParam[i].quickBrakeStrength   = QUICK_BRAKE_STRENGTH_DEFAULT;
    sprintf(g_storedVar.carParam[i].carName, "CAR%d", i);
  }
}

/** Reset all settings (non-car, non-calibration) to factory defaults. */
static void doResetSettings() {
  g_storedVar.viewMode             = VIEW_MODE_LIST;
  g_storedVar.screensaverTimeout   = SCREENSAVER_TIMEOUT_DEFAULT;
  g_storedVar.powerSaveTimeout     = POWER_SAVE_TIMEOUT_DEFAULT;
  g_storedVar.deepSleepTimeout     = DEEP_SLEEP_TIMEOUT_DEFAULT;
  g_storedVar.soundBoot            = SOUND_BOOT_DEFAULT;
  g_storedVar.soundRace            = SOUND_RACE_DEFAULT;
  g_storedVar.gridCarSelectEnabled = 0;
  g_storedVar.raceViewMode         = RACE_VIEW_DEFAULT;
  g_storedVar.language             = LANG_DEFAULT;
  g_storedVar.textCase             = TEXT_CASE_DEFAULT;
  g_storedVar.listFontSize         = FONT_SIZE_DEFAULT;
  g_storedVar.startupDelay         = STARTUP_DELAY_DEFAULT;
  strncpy(g_storedVar.screensaverLine1, SCREENSAVER_LINE1, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine1[SCREENSAVER_TEXT_MAX - 1] = '\0';
  strncpy(g_storedVar.screensaverLine2, SCREENSAVER_LINE2, SCREENSAVER_TEXT_MAX - 1);
  g_storedVar.screensaverLine2[SCREENSAVER_TEXT_MAX - 1] = '\0';
}

/** Reset trigger calibration to factory defaults. */
static void doResetCalibration() {
  g_storedVar.minTrigger_raw = 0;
#if defined(TLE493D_MAG)
  g_storedVar.maxTrigger_raw = 3600;
#else
  g_storedVar.maxTrigger_raw = ACD_RESOLUTION_STEPS;
#endif
}

/**
 * Soft sleep mode. Shows brief message, reduces CPU to 80 MHz, turns display off,
 * suspends motor task. Wakes on encoder button press.
 */
static void showPowerSave() {
  showPowerSave(millis());
}

static void showPowerSave(uint32_t inactivityStartMs) {
  if (isOtaInProgress()) {
    return;
  }

  uint16_t lang = g_storedVar.language;

  /* Brief sleep message */
  obdFill(&g_obd, OBD_WHITE, 1);
  const char* msg = (lang == LANG_NOR) ? "SOVER..." : "SLEEPING...";
  int tw = strlen(msg) * WIDTH8x8;
  obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 28, (char*)msg, FONT_8x8, OBD_BLACK, 1);
  delay(1200);

  /* Sleep mode policy: always disable WiFi/AP while sleeping. */
  stopTimedWiFiPortal();

  /* Safe: motor off */
  HalfBridge_SetPwmDrag(0, 0);
  /* Display off */
  obdPower(&g_obd, 0);
  /* Reduce CPU frequency to save power */
  setCpuFrequencyMhz(80);
  /* Suspend motor control task */
  vTaskSuspend(Task2);

  /* Wait for encoder button press to wake.
   * While waiting, allow deep sleep timeout to take over. */
  uint32_t screensaverMs = (uint32_t)g_storedVar.screensaverTimeout * 1000UL;
  while (digitalRead(ENCODER_BUTTON_PIN) != BUTTON_PRESSED) {
    if (g_storedVar.screensaverTimeout > 0 && g_storedVar.deepSleepTimeout > 0 &&
        millis() - inactivityStartMs > screensaverMs + ((uint32_t)g_storedVar.deepSleepTimeout * 60000UL)) {
      showDeepSleep();  /* Never returns */
    }
    vTaskDelay(50);
  }

  /* Wake up: restore CPU, resume motor task, display on */
  setCpuFrequencyMhz(240);
  vTaskResume(Task2);
  obdPower(&g_obd, 1);
  obdFill(&g_obd, OBD_WHITE, 1);
  delay(200);  /* Debounce */
}

static void showDeepSleep() {
  if (isOtaInProgress()) {
    return;
  }

  uint16_t lang = g_storedVar.language;

  /* Brief power-off message */
  obdFill(&g_obd, OBD_WHITE, 1);
  const char* msg = (lang == LANG_NOR) ? "SLUKKER..." : "POWERING OFF...";
  int tw = strlen(msg) * WIDTH8x8;
  obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 28, (char*)msg, FONT_8x8, OBD_BLACK, 1);
  delay(1200);

  /* Safe shutdown: motor off, display off */
  HalfBridge_SetPwmDrag(0, 0);
  obdPower(&g_obd, 0);

  /* Enter deep sleep - wakes only on power cycle */
  esp_deep_sleep_start();
}

/**
 * Sleep settings submenu: INTERVAL (auto-sleep timeout) and NOW (manual sleep).
 */
static void showSleepSettings() {
  uint16_t lang = g_storedVar.language;

  /* Labels [NOR, ENG, CS, ACD] */
  const char* lblInterval[4] = {"INTERV.",  "INTERVAL", "INTERVAL", "INTERVAL"};
  const char* lblNow[4]      = {"SOV NA",   "SLEEP NOW", "SLEEP NOW", "SLEEP NOW"};
  const char* lblBack[4]     = {"TILBAKE",  "BACK",      "BACK",      "BACK"};
  const char* lblOff[4]      = {"AV",       "OFF",       "OFF",       "OFF"};

  const uint8_t NUM_ITEMS = 3;  /* 0=INTERVAL, 1=NOW, 2=BACK */
  const uint8_t menuFont  = FONT_8x8;
  const uint8_t charWidth = WIDTH8x8;
  const uint8_t lineH     = HEIGHT8x8;

  int8_t  sel      = 0;
  bool    editing  = false;
  uint16_t tmpTimeout = g_storedVar.powerSaveTimeout;
  bool    needRedraw  = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

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

    /* Encoder movement */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (editing) {
        tmpTimeout = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (int8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    /* Encoder button */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        /* Confirm new interval */
        g_storedVar.powerSaveTimeout = tmpTimeout;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(0);
      } else if (sel == 0) {
        /* Enter interval editing */
        editing    = true;
        tmpTimeout = g_storedVar.powerSaveTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, POWER_SAVE_TIMEOUT_MAX, false);
        g_rotaryEncoder.reset(tmpTimeout);
      } else if (sel == 1) {
        /* Sleep NOW */
        showPowerSave();
        lastInteraction = millis();
        screensaverActive = false;
        needRedraw = true;
      } else {
        /* BACK */
        break;
      }
      needRedraw = true;
    }

    /* Brake button = cancel/back */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (editing) {
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(0);
        needRedraw = true;
        delay(200);
      } else {
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    }

    /* Draw */
    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);

      /* Row 0: INTERVAL + value */
      bool s0 = (!editing && sel == 0);
      obdWriteString(&g_obd, 0, 0, 0, (char*)lblInterval[lang], menuFont, s0 ? OBD_WHITE : OBD_BLACK, 1);
      char valStr[8];
      uint16_t disp = editing ? tmpTimeout : g_storedVar.powerSaveTimeout;
      if (disp == 0) snprintf(valStr, sizeof(valStr), "  %s", lblOff[lang]);
      else           snprintf(valStr, sizeof(valStr), "%2dmin", disp);
      uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * charWidth);
      obdWriteString(&g_obd, 0, vx, 0, valStr, menuFont, (s0 || editing) ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 1: NOW */
      bool s1 = (!editing && sel == 1);
      obdWriteString(&g_obd, 0, 0, lineH, (char*)lblNow[lang], menuFont, s1 ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 2: BACK */
      bool s2 = (!editing && sel == 2);
      obdWriteString(&g_obd, 0, 0, 2 * lineH, (char*)lblBack[lang], menuFont, s2 ? OBD_WHITE : OBD_BLACK, 1);

      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }
}

/**
 * Deep sleep settings submenu: INTERVAL (auto deep-sleep timeout) and NOW (manual deep sleep).
 * Uses hardcoded FONT_8x8 regardless of listFontSize.
 */
static void showDeepSleepSettings() {
  uint16_t lang = g_storedVar.language;

  const char* lblInterval[4] = {"INTERV.",    "INTERVAL",   "INTERVAL",   "INTERVAL"};
  const char* lblNow[4]      = {"SLUKK NA",   "SLEEP NOW",  "SLEEP NOW",  "SLEEP NOW"};
  const char* lblBack[4]     = {"TILBAKE",    "BACK",       "BACK",       "BACK"};
  const char* lblOff[4]      = {"AV",         "OFF",        "OFF",        "OFF"};

  const uint8_t NUM_ITEMS = 3;  /* 0=INTERVAL, 1=NOW, 2=BACK */

  int8_t  sel      = 0;
  bool    editing  = false;
  uint16_t tmpTimeout = g_storedVar.deepSleepTimeout;
  bool    needRedraw  = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
  g_rotaryEncoder.reset(0);

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
      if (editing) {
        tmpTimeout = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (int8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        g_storedVar.deepSleepTimeout = tmpTimeout;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(0);
      } else if (sel == 0) {
        editing    = true;
        tmpTimeout = g_storedVar.deepSleepTimeout;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        /* 0=OFF, DEEP_SLEEP_TIMEOUT_MIN-DEEP_SLEEP_TIMEOUT_MAX; encode 0 as OFF */
        g_rotaryEncoder.setBoundaries(0, DEEP_SLEEP_TIMEOUT_MAX, false);
        g_rotaryEncoder.reset(tmpTimeout);
      } else if (sel == 1) {
        /* Deep sleep NOW - never returns */
        showDeepSleep();
        needRedraw = true;
      } else {
        break;
      }
      needRedraw = true;
    }

    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (editing) {
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM_ITEMS - 1, false);
        g_rotaryEncoder.reset(0);
        needRedraw = true;
        delay(200);
      } else {
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    }

    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);

      /* Row 0: INTERVAL + value */
      bool s0 = (!editing && sel == 0);
      obdWriteString(&g_obd, 0, 0, 0, (char*)lblInterval[lang], FONT_8x8, s0 ? OBD_WHITE : OBD_BLACK, 1);
      char valStr[8];
      uint16_t disp = editing ? tmpTimeout : g_storedVar.deepSleepTimeout;
      if (disp == 0) snprintf(valStr, sizeof(valStr), "  %s", lblOff[lang]);
      else           snprintf(valStr, sizeof(valStr), "%2dmin", disp);
      uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * WIDTH8x8);
      obdWriteString(&g_obd, 0, vx, 0, valStr, FONT_8x8, (s0 || editing) ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 1: NOW */
      bool s1 = (!editing && sel == 1);
      obdWriteString(&g_obd, 0, 0, HEIGHT8x8, (char*)lblNow[lang], FONT_8x8, s1 ? OBD_WHITE : OBD_BLACK, 1);

      /* Row 2: BACK */
      bool s2 = (!editing && sel == 2);
      obdWriteString(&g_obd, 0, 0, 2 * HEIGHT8x8, (char*)lblBack[lang], FONT_8x8, s2 ? OBD_WHITE : OBD_BLACK, 1);

      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }
}

/**
 * Sound settings submenu: BOOT (on/off), RACE (on/off), BACK.
 * BOOT controls startup/calibration/on/off sounds.
 * RACE controls the race mode toggle beep.
 */
static void showSoundSettings() {
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

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
static void showWiFiSettings() {
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
  uint16_t tmpMinutes = constrain(g_wifiTimedMinutes, 1, 120);
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
        g_wifiTimedMinutes = constrain(tmpMinutes, 1, 120);
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
        tmpMinutes = constrain(g_wifiTimedMinutes, 1, 120);
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, 120, false);
        g_rotaryEncoder.reset(tmpMinutes);
      } else if (sel == 2) {
        if (isWiFiPortalActive()) {
          stopTimedWiFiPortal();
        } else {
          startTimedWiFiPortal(g_wifiTimedMinutes);
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
      uint16_t shownMinutes = editing ? tmpMinutes : constrain(g_wifiTimedMinutes, 1, 120);
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

    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }
    vTaskDelay(10);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}

/**
 * Power settings submenu: SCRSV, SLEEP, DEEP SLEEP, STARTUP DELAY, BACK.
 * SCRSV/SLEEP/DEEP SLEEP open submenus; STARTUP DELAY is edited inline.
 */
static void showPowerSettings() {
  uint16_t lang = g_storedVar.language;
  const uint8_t NUM = POWER_ITEMS_COUNT;  /* 5: SCRSV, SLEEP, DEEP SLEEP, STARTUP, BACK */
  const uint8_t menuFont = FONT_8x8;
  const uint8_t lineH    = HEIGHT8x8;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
  g_rotaryEncoder.reset(0);

  uint8_t sel = 0;
  bool editing = false;
  uint16_t tmpDelay = g_storedVar.startupDelay;
  uint16_t origDelay = g_storedVar.startupDelay;
  bool needRedraw = true;
  uint32_t lastInteraction = millis();
  bool ssActive = false;
  uint16_t ssEncoderPos = 0;

  auto resumeAfterChildMenu = [&](uint8_t selectedIndex) {
    lastInteraction = millis();
    g_lastEncoderInteraction = lastInteraction;
    ssActive = false;
    ssEncoderPos = g_rotaryEncoder.readEncoder();
    g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
    g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
    g_rotaryEncoder.reset(selectedIndex);
    obdFill(&g_obd, OBD_WHITE, 1);
    needRedraw = true;
  };

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
      if (editing) {
        tmpDelay = (uint16_t)g_rotaryEncoder.readEncoder();
      } else {
        sel = (uint8_t)g_rotaryEncoder.readEncoder();
      }
      needRedraw = true;
    }

    /* Encoder button click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (editing) {
        /* Confirm startup delay */
        g_storedVar.startupDelay = tmpDelay;
        saveEEPROM(g_storedVar);
        editing = false;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
        g_rotaryEncoder.reset(3);
        needRedraw = true;
      } else if (sel == 0) {  /* SCRSV */
        showScreensaverSettings();
        if (g_escapeToMain) break;
        resumeAfterChildMenu(0);
      } else if (sel == 1) {  /* SLEEP */
        showSleepSettings();
        if (g_escapeToMain) break;
        resumeAfterChildMenu(1);
      } else if (sel == 2) {  /* DEEP SLEEP */
        showDeepSleepSettings();
        if (g_escapeToMain) break;
        resumeAfterChildMenu(2);
      } else if (sel == 3) {  /* STARTUP DELAY — enter edit */
        editing = true;
        origDelay = g_storedVar.startupDelay;
        tmpDelay  = g_storedVar.startupDelay;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(STARTUP_DELAY_MIN, STARTUP_DELAY_MAX, false);
        g_rotaryEncoder.reset(tmpDelay);
        needRedraw = true;
      } else {  /* BACK */
        break;
      }
    }

    /* Brake button = cancel edit or back */
    static bool brakeBtnInPower = false;
    static uint32_t lastBrakeBtnPowerTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInPower && millis() - lastBrakeBtnPowerTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInPower = true;
        lastBrakeBtnPowerTime = millis();
        if (editing) {
          g_storedVar.startupDelay = origDelay;
          tmpDelay = origDelay;
          editing = false;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(0, NUM - 1, false);
          g_rotaryEncoder.reset(3);
          needRedraw = true;
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeBtnInPower = false;
    }

    /* Draw menu */
    if (needRedraw) {
      obdFill(&g_obd, OBD_WHITE, 1);
      for (uint8_t i = 0; i < NUM; i++) {
        bool isHighlighted = editing ? (i == 3) : (sel == i);
        obdWriteString(&g_obd, 0, 0, i * lineH, (char*)getPowerMenuName(lang, i),
                       menuFont, isHighlighted ? OBD_WHITE : OBD_BLACK, 1);
        /* Show startup delay value on the right for item 3 */
        if (i == 3) {
          char valStr[7];
          uint16_t disp = editing ? tmpDelay : g_storedVar.startupDelay;
          if (disp == 0) snprintf(valStr, sizeof(valStr), "   %s", (lang == LANG_NOR) ? "AV" : "OFF");
          else           snprintf(valStr, sizeof(valStr), "%3dms", disp * 10);
          uint8_t vx = OLED_WIDTH - (uint8_t)(strlen(valStr) * WIDTH8x8);
          obdWriteString(&g_obd, 0, vx, i * lineH, valStr, menuFont,
                         isHighlighted ? OBD_WHITE : OBD_BLACK, 1);
        }
      }
      needRedraw = false;
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }
}

/**
 * Display settings submenu: VIEW, LANG, CASE, FSIZE, STATUS, BACK.
 * Handles value editing for display-related parameters.
 */
static void showDisplaySettings() {
  initDisplayMenuItems();
  obdFill(&g_obd, OBD_WHITE, 1);

  uint16_t lang = g_storedVar.language;
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, DISPLAY_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(1);

  uint16_t sel = 1;
  uint16_t prevSel = 0;
  MenuState_enum menuState = ITEM_SELECTION;
  uint16_t *valuePtr = NULL;
  uint16_t originalValue = 0;

  uint16_t prevLanguage = g_storedVar.language;
  uint16_t tempLanguage = g_storedVar.language;
  bool isEditingLanguage = false;
  uint16_t prevTextCase = g_storedVar.textCase;
  uint16_t tempTextCase = g_storedVar.textCase;
  bool isEditingTextCase = false;
  uint16_t prevFontSize = g_storedVar.listFontSize;
  uint16_t tempFontSize = g_storedVar.listFontSize;
  bool isEditingFontSize = false;

  uint8_t visibleLines = DISPLAY_ITEMS_COUNT;  /* All items fit on screen with FONT_8x8 */
  uint16_t frameUpper = 1;
  uint16_t frameLower = visibleLines;

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
        }
        delay(10);
        continue;
      }
    }

    /* Encoder button click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (menuState == ITEM_SELECTION) {
        if (sel == DISPLAY_ITEMS_COUNT) {  /* BACK */
          break;
        }
        /* STATUS submenu (item 5, index 4, sel = DISPLAY_ITEMS_COUNT - 1) */
        if (sel == DISPLAY_ITEMS_COUNT - 1) {
          showStatusSettings();
          if (g_escapeToMain) break;
          initDisplayMenuItems();
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(1, DISPLAY_ITEMS_COUNT, false);
          g_rotaryEncoder.reset(sel);
          obdFill(&g_obd, OBD_WHITE, 1);
          prevSel = 0;
          continue;
        }
        /* Value items */
        if (g_settingsMenu.item[sel - 1].value != ITEM_NO_VALUE) {
          if (sel == 2) {  /* LANG */
            isEditingLanguage = true;
            tempLanguage = g_storedVar.language;
            originalValue = g_storedVar.language;
          } else if (sel == 3) {  /* CASE */
            isEditingTextCase = true;
            tempTextCase = g_storedVar.textCase;
            originalValue = g_storedVar.textCase;
          } else if (sel == 4) {  /* FSIZE */
            isEditingFontSize = true;
            tempFontSize = g_storedVar.listFontSize;
            originalValue = g_storedVar.listFontSize;
          } else {
            valuePtr = (uint16_t *)g_settingsMenu.item[sel - 1].value;
            originalValue = *valuePtr;
          }
          menuState = VALUE_SELECTION;
          g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
          if (!isEditingLanguage && !isEditingTextCase && !isEditingFontSize) {
            valuePtr = (uint16_t *)g_settingsMenu.item[sel - 1].value;
          }
          g_rotaryEncoder.setBoundaries(g_settingsMenu.item[sel - 1].minValue,
                                        g_settingsMenu.item[sel - 1].maxValue, false);
          uint16_t resetVal = isEditingLanguage ? tempLanguage :
                              (isEditingTextCase ? tempTextCase :
                              (isEditingFontSize ? tempFontSize : *valuePtr));
          g_rotaryEncoder.reset(resetVal);
        }
      } else {
        /* Confirm edit */
        menuState = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, DISPLAY_ITEMS_COUNT, false);
        g_rotaryEncoder.reset(sel);
        if (isEditingLanguage) { g_storedVar.language = tempLanguage; lang = tempLanguage; isEditingLanguage = false; }
        if (isEditingTextCase) { g_storedVar.textCase = tempTextCase; isEditingTextCase = false; }
        if (isEditingFontSize) { g_storedVar.listFontSize = tempFontSize; isEditingFontSize = false; }
        saveEEPROM(g_storedVar);
        if (g_storedVar.language != prevLanguage || g_storedVar.textCase != prevTextCase || g_storedVar.listFontSize != prevFontSize) {
          initMenuItems();
          initSettingsMenuItems();
          initDisplayMenuItems();  /* Must be last: shared g_settingsMenu is overwritten by initSettingsMenuItems */
          prevLanguage = g_storedVar.language;
          prevTextCase = g_storedVar.textCase;
          prevFontSize = g_storedVar.listFontSize;
          /* visibleLines stays DISPLAY_ITEMS_COUNT — submenu always uses FONT_8x8 */
          frameUpper = 1;
          frameLower = visibleLines;
          obdFill(&g_obd, OBD_WHITE, 1);
          lang = g_storedVar.language;
        }
      }
      delay(200);
    }

    /* Encoder movement */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      if (menuState == ITEM_SELECTION) {
        sel = g_rotaryEncoder.readEncoder();
      } else {
        if (isEditingLanguage) {
          tempLanguage = g_rotaryEncoder.readEncoder();
        } else if (isEditingTextCase) {
          uint16_t newTC = g_rotaryEncoder.readEncoder();
          if (newTC != tempTextCase) {
            tempTextCase = newTC;
            obdFill(&g_obd, OBD_WHITE, 1);
          }
        } else if (isEditingFontSize) {
          tempFontSize = g_rotaryEncoder.readEncoder();
        } else {
          *valuePtr = g_rotaryEncoder.readEncoder();
        }
      }
    }

    /* Brake button = cancel edit or back */
    static bool brakeBtnInDisplay = false;
    static uint32_t lastBrakeBtnDisplayTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInDisplay && millis() - lastBrakeBtnDisplayTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInDisplay = true;
        lastBrakeBtnDisplayTime = millis();
        lastInteraction = millis();
        if (menuState == VALUE_SELECTION) {
          if (isEditingLanguage) { tempLanguage = originalValue; g_storedVar.language = originalValue; isEditingLanguage = false; }
          else if (isEditingTextCase) { tempTextCase = originalValue; g_storedVar.textCase = originalValue; isEditingTextCase = false; }
          else if (isEditingFontSize) { tempFontSize = originalValue; g_storedVar.listFontSize = originalValue; isEditingFontSize = false; }
          else if (valuePtr != NULL) { *valuePtr = originalValue; }
          menuState = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(1, DISPLAY_ITEMS_COUNT, false);
          g_rotaryEncoder.reset(sel);
          obdFill(&g_obd, OBD_WHITE, 1);
        } else {
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
      }
    } else {
      brakeBtnInDisplay = false;
    }

    /* Frame scrolling */
    if (menuState == ITEM_SELECTION) {
      if (sel > frameLower) {
        frameLower = sel;
        frameUpper = frameLower - visibleLines + 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      } else if (sel < frameUpper) {
        frameUpper = sel;
        frameLower = frameUpper + visibleLines - 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Display — submenu always uses FONT_8x8 regardless of global listFontSize */
    const uint8_t menuFont  = FONT_8x8;
    const uint8_t charWidth = WIDTH8x8;
    const uint8_t lineHeight = HEIGHT8x8;

    for (uint8_t i = 0; i < visibleLines; i++) {
      uint16_t itemIdx = frameUpper - 1 + i;
      if (itemIdx >= DISPLAY_ITEMS_COUNT) break;

      bool isSelected = (sel - frameUpper == i && menuState == ITEM_SELECTION);
      obdWriteString(&g_obd, 0, 0, i * lineHeight, (char*)getDisplayMenuName(lang, itemIdx),
                     menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);

      if (g_settingsMenu.item[itemIdx].value != ITEM_NO_VALUE) {
        bool isValueSel = (sel - frameUpper == i && menuState == VALUE_SELECTION);
        uint16_t value = *(uint16_t *)(g_settingsMenu.item[itemIdx].value);

        if (itemIdx == 0) {  /* VIEW */
          sprintf(msgStr, "%6s", getViewModeLabel(g_storedVar.language, value));
        } else if (itemIdx == 1) {  /* LANG */
          uint16_t dispLang = (isEditingLanguage && isValueSel) ? tempLanguage : value;
          sprintf(msgStr, "%3s", LANG_LABELS[dispLang]);
        } else if (itemIdx == 2) {  /* CASE */
          uint16_t dispTC = (isEditingTextCase && isValueSel) ? tempTextCase : value;
          sprintf(msgStr, "%6s", TEXT_CASE_LABELS[g_storedVar.language][dispTC]);
        } else if (itemIdx == 3) {  /* FSIZE */
          uint16_t dispFS = (isEditingFontSize && isValueSel) ? tempFontSize : value;
          sprintf(msgStr, "%5s", FONT_SIZE_LABELS[g_storedVar.language][dispFS]);
        } else {
          sprintf(msgStr, "%3d", value);
        }

        int textWidth = strlen(msgStr) * charWidth;
        obdWriteString(&g_obd, 0, OLED_WIDTH - textWidth, i * lineHeight, msgStr,
                       menuFont, isValueSel ? OBD_WHITE : OBD_BLACK, 1);
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }
}

/**
 * Reset submenu. Items: CAR, SETTINGS, CALIBRATION, ALL, BACK.
 * Each destructive action requires double-click confirmation.
 */
void showResetSubmenu() {
  const uint8_t RS_ITEMS = 5;
  uint16_t lang = g_storedVar.language;

  const char* rowNames[RS_ITEMS];
  if (lang == LANG_NOR) {
    rowNames[0] = "Bil";
    rowNames[1] = "Innstillinger";
    rowNames[2] = "Kalibrering";
    rowNames[3] = "Alt";
    rowNames[4] = "Tilbake";
  } else {
    rowNames[0] = "Car";
    rowNames[1] = "Settings";
    rowNames[2] = "Calibration";
    rowNames[3] = "Everything";
    rowNames[4] = "Back";
  }

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, RS_ITEMS, false);
  g_rotaryEncoder.reset(1);

  uint8_t sel      = 1;
  uint8_t prevSel  = 0xFF;
  bool forceRedraw = true;
  uint32_t lastInteraction = millis();
  bool screensaverActive = false;
  uint16_t screensaverEncoderPos = 0;

  static bool brakeInReset = false;
  static uint32_t lastBrakeReset = 0;

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
        forceRedraw = true;
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
          forceRedraw = true;
        }
        delay(10);
        continue;
      }
    }

    if (!wakeUp && screensaverActive) {
      if (serviceIdlePowerTransitions(&lastInteraction, &screensaverActive)) {
        obdFill(&g_obd, OBD_WHITE, 1);
        forceRedraw = true;
      }
      delay(10);
      continue;
    }

    /* Brake exits */
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      lastInteraction = millis();
      if (!brakeInReset && millis() - lastBrakeReset > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeInReset = true;
        lastBrakeReset = millis();
        while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
        break;
      }
    } else {
      brakeInReset = false;
    }

    /* Encoder scroll */
    if (g_rotaryEncoder.encoderChanged()) {
      lastInteraction = millis();
      sel = (uint8_t)g_rotaryEncoder.readEncoder();
      forceRedraw = true;
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (sel == RS_ITEMS) break;  /* BACK */

      bool confirmed = resetConfirm(rowNames[sel - 1]);
      lastInteraction = millis();
      screensaverActive = false;
      if (confirmed) {
        obdFill(&g_obd, OBD_WHITE, 1);
        const char* doneText = (lang == LANG_NOR) ? "NULLSTILT!" : "RESET DONE!";
        int tw = strlen(doneText) * WIDTH12x16;
        obdWriteString(&g_obd, 0, (OLED_WIDTH - tw) / 2, 24, (char *)doneText, FONT_12x16, OBD_BLACK, 1);
        delay(1500);

        if (sel == 4) {
          /* Full factory reset: wipe flash, reboot into first-boot path */
          g_pref.begin("stored_var", false);
          g_pref.clear();
          g_pref.end();
          ESP.restart();
        }

        if (sel == 1) { doResetCar(); }
        else if (sel == 2) { doResetSettings(); }
        else if (sel == 3) { doResetCalibration(); }

        saveEEPROM(g_storedVar);
        initMenuItems();
        initSettingsMenuItems();

        if (sel == 3) {
          g_pref.begin("stored_var", false);
          g_pref.putBool("force_calib", true);
          g_pref.end();
          ESP.restart();
        }
      }

      /* Re-init encoder and redraw submenu */
      g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
      g_rotaryEncoder.setBoundaries(1, RS_ITEMS, false);
      g_rotaryEncoder.reset(sel);
      obdFill(&g_obd, OBD_WHITE, 1);
      prevSel     = 0xFF;
      forceRedraw = true;
    }

    /* Draw */
    if (forceRedraw || sel != prevSel) {
      forceRedraw = false;
      prevSel = sel;
      for (uint8_t idx = 0; idx < RS_ITEMS; idx++) {
        uint8_t yPx     = idx * HEIGHT8x8;
        bool isSelected = (sel == idx + 1);
        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowNames[idx], FONT_8x8,
                       isSelected ? OBD_WHITE : OBD_BLACK, 1);
      }
    }

    /* Long press = escape to main for race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }

  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * Quick Brake submenu.
 * Opened by clicking the QB item in the main menu.
 * Items: QB (ON/OFF), QB_TH (threshold %), QB_ST (strength %), BACK.
 * The main menu still shows ON/OFF for the QB item at a glance.
 */
void showQuickBrakeMenu() {
  const uint8_t QB_ITEMS = 4;  /* QB, QB_TH, QB_ST, BACK */
  uint8_t lang = g_storedVar.language;

  const char* rowNames[QB_ITEMS];
  if (lang == LANG_NOR) {
    rowNames[0] = "Aktiv";
    rowNames[1] = "Terskel";
    rowNames[2] = "Styrke";
    rowNames[3] = "Tilbake";
  } else {
    rowNames[0] = "Enable";
    rowNames[1] = "Threshold";
    rowNames[2] = "Strength";
    rowNames[3] = "Back";
  }

  obdFill(&g_obd, OBD_WHITE, 1);
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
  g_rotaryEncoder.reset(1);

  uint8_t sel            = 1;
  uint8_t prevSel        = 0xFF;
  MenuState_enum state   = ITEM_SELECTION;
  uint16_t origValue     = 0;
  bool forceRedraw       = true;

  uint32_t lastInteraction   = millis();
  bool screensaverActive     = false;
  uint16_t screensaverEncPos = 0;

  static bool brakeInQB = false;
  static uint32_t lastBrakeQB = 0;

  while (true) {
    /* Screensaver */
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;
    bool wakeUp = false;
    if (screensaverActive) {
      uint16_t ep = g_rotaryEncoder.readEncoder();
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
          screensaverEncPos = g_rotaryEncoder.readEncoder();
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
          if (sel == 1) g_storedVar.carParam[g_carSel].quickBrakeEnabled  = origValue;
          else if (sel == 2) g_storedVar.carParam[g_carSel].quickBrakeThreshold = origValue;
          else if (sel == 3) g_storedVar.carParam[g_carSel].quickBrakeStrength  = origValue;
          state = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
          g_rotaryEncoder.reset(sel);
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
      uint16_t ep = (uint16_t)g_rotaryEncoder.readEncoder();
      if (state == ITEM_SELECTION) {
        sel = (uint8_t)ep;
        forceRedraw = true;
      } else {
        /* Live update while editing */
        if (sel == 1) g_storedVar.carParam[g_carSel].quickBrakeEnabled  = ep;
        else if (sel == 2) g_storedVar.carParam[g_carSel].quickBrakeThreshold = ep;
        else if (sel == 3) g_storedVar.carParam[g_carSel].quickBrakeStrength  = ep;
        forceRedraw = true;
      }
    }

    /* Encoder click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastInteraction = millis();
      if (state == ITEM_SELECTION) {
        if (sel == QB_ITEMS) {
          /* BACK */
          while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) { vTaskDelay(5); }
          break;
        }
        /* Enter value edit */
        uint16_t minV = 0, maxV = 1, curV = 0;
        if (sel == 1) { curV = g_storedVar.carParam[g_carSel].quickBrakeEnabled;  minV = 0; maxV = 1; }
        else if (sel == 2) { curV = g_storedVar.carParam[g_carSel].quickBrakeThreshold; minV = 0; maxV = QUICK_BRAKE_THRESHOLD_MAX; }
        else if (sel == 3) { curV = g_storedVar.carParam[g_carSel].quickBrakeStrength;  minV = 0; maxV = QUICK_BRAKE_STRENGTH_MAX; }
        origValue = curV;
        g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
        g_rotaryEncoder.setBoundaries(minV, maxV, false);
        g_rotaryEncoder.reset(curV);
        state       = VALUE_SELECTION;
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      } else {
        /* Confirm value */
        saveEEPROM(g_storedVar);
        state = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, QB_ITEMS, false);
        g_rotaryEncoder.reset(sel);
        obdFill(&g_obd, OBD_WHITE, 1);
        prevSel     = 0xFF;
        forceRedraw = true;
      }
    }

    /* Draw */
    if (forceRedraw || sel != prevSel) {
      forceRedraw = false;
      prevSel = sel;

      for (uint8_t idx = 0; idx < QB_ITEMS; idx++) {
        uint8_t yPx       = idx * HEIGHT8x8;
        bool isSelected   = (sel == idx + 1);
        bool inEdit       = (state == VALUE_SELECTION && isSelected);

        obdWriteString(&g_obd, 0, 0, yPx, (char *)rowNames[idx], FONT_8x8,
                       (isSelected && state == ITEM_SELECTION) ? OBD_WHITE : OBD_BLACK, 1);

        /* Value right-justified */
        char vbuf[8];
        if (idx == 0) {
          uint16_t v = g_storedVar.carParam[g_carSel].quickBrakeEnabled;
          snprintf(vbuf, sizeof(vbuf), "%3s", getOnOffLabel(lang, v ? 1 : 0));
        } else if (idx == 1) {
          snprintf(vbuf, sizeof(vbuf), "%3d%%", g_storedVar.carParam[g_carSel].quickBrakeThreshold);
        } else if (idx == 2) {
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
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    vTaskDelay(10);
  }

  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);

  /* Reset encoder for main menu */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
}


/* ---- Self-test helpers ---- */

/* Draw the step header: progress line + bold step name */
static void selfTestStep(uint8_t step, uint8_t total, const char* name) {
  obdFill(&g_obd, OBD_WHITE, 1);
  char hdr[22];
  sprintf(hdr, "Self-Test   %d/%d", step, total);
  obdWriteString(&g_obd, 0, 0, 0 * HEIGHT8x8, hdr, FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 1 * HEIGHT8x8, (char*)name, FONT_8x8, OBD_BLACK, 1);
}

/* Draw PASS/FAIL at bottom-right; optionally draw "enc>" prompt bottom-left */
static void selfTestResult(bool pass, bool showPrompt = false) {
  if (showPrompt) {
    obdWriteString(&g_obd, 0, 0, 7 * HEIGHT8x8, (char*)"enc >", FONT_6x8, OBD_BLACK, 1);
  }
  obdWriteString(&g_obd, 0, 80, 7 * HEIGHT8x8,
    pass ? (char*)"  PASS" : (char*)"  FAIL", FONT_6x8, OBD_BLACK, 1);
}

/* Block until encoder button clicked or brake button pressed+released.
 * Returns true if encoder button (= confirm), false if brake (= skip/exit). */
static bool selfTestWaitEnc() {
  while (true) {
    if (g_rotaryEncoder.isEncoderButtonClicked()) return true;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) vTaskDelay(1);
      return false;
    }
    vTaskDelay(1);
  }
}

/**
 * @brief Interactive hardware self-test sequence (7 steps).
 * @details Checks: display pixels, buzzer, encoder rotation, encoder button,
 *          brake button, trigger sensor, and input voltage.
 *          Launch by holding brake button at startup, or via Settings > Test.
 */
static void showSelfTest() {
  const uint8_t TOTAL = 9;
  bool results[TOTAL] = {};
  const char* testNames[TOTAL] = {
    "Display", "Buzzer ", "Enc Rot",
    "Enc Btn", "Brk Btn", "Trigger",
    "NVS    ", "Motor  ", "Voltage"
  };
  char line[22];

  /* Drain pending encoder clicks; wait for brake button release from startup hold */
  while (g_rotaryEncoder.isEncoderButtonClicked()) {}
  while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) vTaskDelay(1);
  delay(80);

  /* ======== Intro screen ======== */
  obdFill(&g_obd, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, centerX8x8("Self-Test"), 0 * HEIGHT8x8, (char*)"Self-Test", FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  2 * HEIGHT8x8, (char*)"9 hardware checks", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  4 * HEIGHT8x8, (char*)"Enc btn = PASS/next", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  5 * HEIGHT8x8, (char*)"Brk btn = FAIL/skip", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0,  7 * HEIGHT8x8, (char*)"enc >   (start)", FONT_6x8, OBD_BLACK, 1);
  selfTestWaitEnc();

  /* ======== Step 1: Display (auto-pass) ======== */
  /* Phase A: all pixels ON — verify no dead/stuck-off pixels */
  obdFill(&g_obd, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 25, 3 * HEIGHT8x8, (char*)"All pixels ON", FONT_6x8, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, 16, 6 * HEIGHT8x8, (char*)"1/9 Display", FONT_6x8, OBD_WHITE, 1);
  delay(2000);
  /* Phase B: all pixels OFF — verify no stuck-on pixels */
  obdFill(&g_obd, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, 22, 3 * HEIGHT8x8, (char*)"All pixels OFF", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 16, 6 * HEIGHT8x8, (char*)"1/9 Display", FONT_6x8, OBD_BLACK, 1);
  delay(2000);
  results[0] = true;  /* auto-pass: screen is visibly working */

  /* ======== Step 2: Buzzer ======== */
  selfTestStep(2, TOTAL, "Buzzer");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Listen for tone.", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 5 * HEIGHT8x8, (char*)"Enc btn = heard it", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, (char*)"Brk btn = nothing", FONT_6x8, OBD_BLACK, 1);
  sound(NOTE_A, 250);
  delay(80);
  sound(NOTE_C, 200);
  delay(100);
  while (g_rotaryEncoder.isEncoderButtonClicked()) {}
  results[1] = selfTestWaitEnc();
  selfTestResult(results[1]);
  delay(1000);

  /* ======== Step 3: Encoder Rotate ======== */
  g_rotaryEncoder.setAcceleration(0);
  g_rotaryEncoder.setBoundaries(-50, 50, false);
  g_rotaryEncoder.reset(0);
  while (g_rotaryEncoder.isEncoderButtonClicked()) {}
  selfTestStep(3, TOTAL, "Enc Rotate");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Rotate encoder...", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"(auto-pass at 3)", FONT_6x8, OBD_BLACK, 1);
  int16_t encPeak = 0;
  while (abs(encPeak) < 3) {
    int16_t pos = (int16_t)g_rotaryEncoder.readEncoder();
    if (abs(pos) > abs(encPeak)) encPeak = pos;
    sprintf(line, "Clicks: %+d      ", pos);
    obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, line, FONT_6x8, OBD_BLACK, 1);
    vTaskDelay(10);
  }
  results[2] = true;
  selfTestResult(true);
  delay(1500);

  /* ======== Step 4: Encoder Button ======== */
  while (g_rotaryEncoder.isEncoderButtonClicked()) {}
  selfTestStep(4, TOTAL, "Enc Button");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Press encoder btn.", FONT_6x8, OBD_BLACK, 1);
  while (!g_rotaryEncoder.isEncoderButtonClicked()) vTaskDelay(1);
  results[3] = true;
  selfTestResult(true);
  delay(1500);

  /* ======== Step 5: Brake Button ======== */
  while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) vTaskDelay(1);
  selfTestStep(5, TOTAL, "Brk Button");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Press brake btn.", FONT_6x8, OBD_BLACK, 1);
  while (digitalRead(BUTT_PIN) != BUTTON_PRESSED) vTaskDelay(1);
  while (digitalRead(BUTT_PIN) == BUTTON_PRESSED) vTaskDelay(1);
  results[4] = true;
  selfTestResult(true);
  delay(1500);

  /* ======== Step 6: Trigger / Magnetic Sensor ======== */
  selfTestStep(6, TOTAL, "Trigger");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Pull trigger fully.", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"Hold 1s, release.", FONT_6x8, OBD_BLACK, 1);
  int16_t trigBase = HAL_ReadTriggerRaw();
  int16_t trigPeak = 0;
  uint32_t trigStart_ms = millis();
  while (millis() - trigStart_ms < 15000) {
    int16_t raw = HAL_ReadTriggerRaw();
    int16_t delta = abs(raw - trigBase);
    if (delta > trigPeak) trigPeak = delta;
    uint8_t secLeft = (uint8_t)((15000 - (millis() - trigStart_ms)) / 1000);
    sprintf(line, "d:%4d  %2ds left ", trigPeak, secLeft);
    obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, line, FONT_6x8, OBD_BLACK, 1);
    if (trigPeak > 200) { results[5] = true; break; }
    vTaskDelay(20);
  }
  selfTestResult(results[5], !results[5]);
  if (results[5]) {
    delay(1500);
  } else {
    selfTestWaitEnc();
  }

  /* ======== Step 7: NVS / Flash ======== */
  selfTestStep(7, TOTAL, "NVS");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Write-read check...", FONT_6x8, OBD_BLACK, 1);
  {
    const uint32_t MAGIC = 0xC0FFEE42u;
    g_pref.begin("selftest", false);
    g_pref.putUInt("magic", MAGIC);
    uint32_t rb = g_pref.getUInt("magic", 0);
    g_pref.clear();
    g_pref.end();
    results[6] = (rb == MAGIC);
  }
  selfTestResult(results[6]);
  delay(1500);

  /* ======== Step 8: Motor Controller (idle current sense) ======== */
  selfTestStep(8, TOTAL, "Motor ctrl");
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, (char*)"Idle current check", FONT_6x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 4 * HEIGHT8x8, (char*)"(no PWM applied)", FONT_6x8, OBD_BLACK, 1);
  uint16_t idleCurrent_mA = HAL_ReadMotorCurrent();
  bool motorDiagOk = (HalfBridge_GetDiagnosis() == 0);  /* 0 = NO_ERROR */
  results[7] = (idleCurrent_mA < 500) && motorDiagOk;
  sprintf(line, "Idle: %d mA", idleCurrent_mA);
  obdWriteString(&g_obd, 0, 0, 6 * HEIGHT8x8, line, FONT_6x8, OBD_BLACK, 1);
  selfTestResult(results[7]);
  delay(3000);

  /* ======== Step 9: Voltage ======== */
  selfTestStep(9, TOTAL, "Voltage");
  uint16_t vin_mV = HAL_ReadVoltageDivider(AN_VIN_DIV, RVIFBL, RVIFBH);
  results[8] = vin_mV > 2500;
  sprintf(line, "%d mV", vin_mV);
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT8x8, line, FONT_8x8, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 0, 5 * HEIGHT8x8,
    results[8] ? (char*)"OK  (>2500mV)" : (char*)"LOW (<2500mV)", FONT_6x8, OBD_BLACK, 1);
  selfTestResult(results[8], true);
  selfTestWaitEnc();

  /* ======== Summary: two pages (5 + 4 tests) ======== */
  uint8_t passCount = 0;
  for (uint8_t i = 0; i < TOTAL; i++) if (results[i]) passCount++;

  /* Page 1: tests 0-4 */
  obdFill(&g_obd, OBD_WHITE, 1);
  sprintf(line, "Summary %d/%d  (1/2)", passCount, TOTAL);
  obdWriteString(&g_obd, 0, 0, 0, line, FONT_6x8, OBD_BLACK, 1);
  for (uint8_t i = 0; i < 5; i++) {
    sprintf(line, "%-7s  %s", testNames[i], results[i] ? "PASS" : "FAIL");
    obdWriteString(&g_obd, 0, 0, (i + 1) * HEIGHT8x8, line, FONT_6x8, OBD_BLACK, 1);
  }
  obdWriteString(&g_obd, 0, 0, 7 * HEIGHT8x8, (char*)"enc >   (next)", FONT_6x8, OBD_BLACK, 1);
  selfTestWaitEnc();

  /* Page 2: tests 5-8 */
  obdFill(&g_obd, OBD_WHITE, 1);
  sprintf(line, "Summary %d/%d  (2/2)", passCount, TOTAL);
  obdWriteString(&g_obd, 0, 0, 0, line, FONT_6x8, OBD_BLACK, 1);
  for (uint8_t i = 5; i < TOTAL; i++) {
    sprintf(line, "%-7s  %s", testNames[i], results[i] ? "PASS" : "FAIL");
    obdWriteString(&g_obd, 0, 0, (i - 4) * HEIGHT8x8, line, FONT_6x8, OBD_BLACK, 1);
  }
  obdWriteString(&g_obd, 0, 0, 7 * HEIGHT8x8, (char*)"enc >   (exit)", FONT_6x8, OBD_BLACK, 1);
  selfTestWaitEnc();

  /* Restore encoder for main menu */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
  obdFill(&g_obd, OBD_WHITE, 1);
}

/**
 * Show scrollable device information screen.
 * Includes firmware/data versions, chip details, WiFi/BT MAC addresses and build info.
 */
static void showAboutScreen() {
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

  static bool aboutBtnHeld = false;
  aboutBtnHeld = false;
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

/**
 * Show the Settings submenu.
 * Items: POWER (submenu), DISPLAY (submenu), SOUND, WIFI, USB, RESET, ABOUT, BACK.
 */
void showSettingsMenu() {
  initSettingsMenuItems();
  obdFill(&g_obd, OBD_WHITE, 1);
  g_inSettingsMenu = true;

  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, SETTINGS_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(1);

  uint16_t settingsSelector = 1;
  uint16_t prevSettingsSelector = 0;
  MenuState_enum settingsMenuState = ITEM_SELECTION;
  uint16_t *settingsValuePtr = NULL;
  uint16_t originalSettingsValue = 0;

  uint16_t frameUpper = 1;
  uint8_t visibleLines = getMenuLines();
  uint16_t frameLower = visibleLines;

  uint32_t lastSettingsInteraction = millis();
  bool settingsScreensaverActive = false;
  uint16_t settingsScreensaverEncoderPos = 0;

  auto resumeAfterSettingsChild = [&]() {
    lastSettingsInteraction = millis();
    g_lastEncoderInteraction = lastSettingsInteraction;
    settingsScreensaverActive = false;
    settingsScreensaverEncoderPos = g_rotaryEncoder.readEncoder();
    initSettingsMenuItems();
    g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
    g_rotaryEncoder.setBoundaries(1, SETTINGS_ITEMS_COUNT, false);
    g_rotaryEncoder.reset(settingsSelector);
    obdFill(&g_obd, OBD_WHITE, 1);
    prevSettingsSelector = 0;
  };

  while (true) {
    uint8_t throttle_pct = (g_escVar.trigger_norm * 100) / THROTTLE_NORMALIZED;

    /* Screensaver wake-up */
    bool wakeUpTriggered = false;
    if (settingsScreensaverActive) {
      uint16_t currentEncoderPos = g_rotaryEncoder.readEncoder();
      if (throttle_pct >= SCREENSAVER_WAKEUP_THRESHOLD ||
          currentEncoderPos != settingsScreensaverEncoderPos ||
          digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
        wakeUpTriggered = true;
        settingsScreensaverActive = false;
        lastSettingsInteraction = millis();
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    /* Screensaver timeout */
    if (consumeScreensaverWakeInput(wakeUpTriggered)) { continue; }

    if (!wakeUpTriggered && g_storedVar.screensaverTimeout > 0 &&
        millis() - lastSettingsInteraction > (g_storedVar.screensaverTimeout * 1000UL)) {
      if (throttle_pct < SCREENSAVER_WAKEUP_THRESHOLD) {
        if (!settingsScreensaverActive) {
          settingsScreensaverActive = true;
          settingsScreensaverEncoderPos = g_rotaryEncoder.readEncoder();
          showScreensaver();
        }
        if (serviceIdlePowerTransitions(&lastSettingsInteraction, &settingsScreensaverActive)) {
          obdFill(&g_obd, OBD_WHITE, 1);
        }
        delay(10);
        continue;
      }
    }

    /* Encoder button click */
    if (g_rotaryEncoder.isEncoderButtonClicked()) {
      lastSettingsInteraction = millis();
      if (settingsMenuState == ITEM_SELECTION) {
        /* BACK */
        if (settingsSelector == SETTINGS_ITEMS_COUNT) { break; }
        /* POWER submenu */
        if (settingsSelector == 1) {
          showPowerSettings();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* DISPLAY submenu */
        if (settingsSelector == 2) {
          showDisplaySettings();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* SOUND submenu */
        if (settingsSelector == 3) {
          showSoundSettings();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* WIFI submenu */
        if (settingsSelector == SETTINGS_ITEMS_COUNT - 5) {
          showWiFiSettings();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* USB */
        if (settingsSelector == SETTINGS_ITEMS_COUNT - 4) {
          showUSBPortalScreen();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* RESET */
        if (settingsSelector == SETTINGS_ITEMS_COUNT - 3) {
          showResetSubmenu();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* TEST */
        if (settingsSelector == SETTINGS_ITEMS_COUNT - 2) {
          showSelfTest();
          if (g_escapeToMain) break;
          resumeAfterSettingsChild();
          continue;
        }
        /* ABOUT */
        if (settingsSelector == SETTINGS_ITEMS_COUNT - 1) {
          showAboutScreen();
          resumeAfterSettingsChild();
          continue;
        }
        /* Value items (SOUND=3, DELAY=4) */
        if (g_settingsMenu.item[settingsSelector - 1].value != ITEM_NO_VALUE) {
          settingsValuePtr = (uint16_t *)g_settingsMenu.item[settingsSelector - 1].value;
          originalSettingsValue = *settingsValuePtr;
          settingsMenuState = VALUE_SELECTION;
          g_rotaryEncoder.setAcceleration(SEL_ACCELERATION);
          g_rotaryEncoder.setBoundaries(g_settingsMenu.item[settingsSelector - 1].minValue,
                                        g_settingsMenu.item[settingsSelector - 1].maxValue, false);
          g_rotaryEncoder.reset(*settingsValuePtr);
        }
      } else {
        /* Confirm edit */
        settingsMenuState = ITEM_SELECTION;
        g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
        g_rotaryEncoder.setBoundaries(1, SETTINGS_ITEMS_COUNT, false);
        g_rotaryEncoder.reset(settingsSelector);
        saveEEPROM(g_storedVar);
      }
      delay(200);
    }

    /* Encoder movement */
    if (g_rotaryEncoder.encoderChanged()) {
      lastSettingsInteraction = millis();
      if (settingsMenuState == ITEM_SELECTION) {
        settingsSelector = g_rotaryEncoder.readEncoder();
      } else {
        *settingsValuePtr = g_rotaryEncoder.readEncoder();
      }
    }

    /* Brake button = cancel edit or exit */
    static bool brakeBtnInSettings = false;
    static uint32_t lastBrakeBtnSettingsTime = 0;
    if (digitalRead(BUTT_PIN) == BUTTON_PRESSED) {
      if (!brakeBtnInSettings && millis() - lastBrakeBtnSettingsTime > BUTTON_SHORT_PRESS_DEBOUNCE_MS) {
        brakeBtnInSettings = true;
        lastBrakeBtnSettingsTime = millis();
        lastSettingsInteraction = millis();
        if (settingsMenuState == VALUE_SELECTION) {
          if (settingsValuePtr != NULL) { *settingsValuePtr = originalSettingsValue; }
          settingsMenuState = ITEM_SELECTION;
          g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
          g_rotaryEncoder.setBoundaries(1, SETTINGS_ITEMS_COUNT, false);
          g_rotaryEncoder.reset(settingsSelector);
          obdFill(&g_obd, OBD_WHITE, 1);
        } else {
          break;
        }
      }
    } else {
      brakeBtnInSettings = false;
    }

    /* Long press encoder button = escape to main + race mode toggle */
    if (checkRaceModeEscape()) { g_escapeToMain = true; break; }

    /* Frame scrolling */
    if (settingsMenuState == ITEM_SELECTION) {
      if (settingsSelector > frameLower) {
        frameLower = settingsSelector;
        frameUpper = frameLower - visibleLines + 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      } else if (settingsSelector < frameUpper) {
        frameUpper = settingsSelector;
        frameLower = frameUpper + visibleLines - 1;
        obdFill(&g_obd, OBD_WHITE, 1);
      }
    }

    uint8_t menuFont = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? FONT_8x8 : FONT_12x16;
    uint8_t charWidth = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? WIDTH8x8 : WIDTH12x16;
    uint8_t lineHeight = (g_storedVar.listFontSize == FONT_SIZE_SMALL) ? 8 : HEIGHT12x16;

    for (uint8_t i = 0; i < visibleLines; i++) {
      uint16_t itemIndex = frameUpper - 1 + i;
      if (itemIndex >= SETTINGS_ITEMS_COUNT) break;

      bool isSelected = (settingsSelector - frameUpper == i && settingsMenuState == ITEM_SELECTION);
      obdWriteString(&g_obd, 0, 0, i * lineHeight, g_settingsMenu.item[itemIndex].name,
                     menuFont, isSelected ? OBD_WHITE : OBD_BLACK, 1);

      if (g_settingsMenu.item[itemIndex].value != ITEM_NO_VALUE) {
        bool isValueSelected = (settingsSelector - frameUpper == i && settingsMenuState == VALUE_SELECTION);
        uint16_t lang = g_storedVar.language;
        uint16_t value = *(uint16_t *)(g_settingsMenu.item[itemIndex].value);
        sprintf(msgStr, "%3d", value);
        int textWidth = strlen(msgStr) * charWidth;
        obdWriteString(&g_obd, 0, OLED_WIDTH - textWidth, i * lineHeight, msgStr,
                       menuFont, isValueSelected ? OBD_WHITE : OBD_BLACK, 1);
      }
    }

    vTaskDelay(10);
  }

  g_inSettingsMenu = false;
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
  g_escVar.encoderPos = g_encoderMainSelector;
  saveEEPROM(g_storedVar);
  obdFill(&g_obd, OBD_WHITE, 1);
}


/**
 * Take an ADC raw value where the max and min has been recorded and returns the value scaled
 * 
 * @param raw The raw ADC value
 * @param minIn The lowest reading of the ADC
 * @param maxIn The highest reading of the ADC
 * @param normalizedMax The new max value of the scale
 * @param isReversed Whether the throttle is reversed or not
 * @return The raw ADC value scaled from 0 to normalizedMax
 */
uint16_t normalizeAndClamp(uint16_t raw, uint16_t minIn, uint16_t maxIn, uint16_t normalizedMax, bool isReversed)
{
  uint16_t retVal = 0;  /* From 0 to normalizedMax */

  if (maxIn == minIn)  /* If maxIn == minIn avoid division by 0 */
  {
    retVal = 0;
  }
  else
  {
    raw = constrain(raw, minIn, maxIn); /* Make sure the raw value is constrained between minIn and maxIn */

    if (isReversed == true)  /* If throttle is reversed (it goes to low values when pressed) */
    {
      raw = abs(maxIn - raw);
    }
    else
    {
      raw = abs(raw - minIn);
    }

    retVal = ((uint32_t)raw * normalizedMax) / (maxIn - minIn); /* Scale the raw reading */
  }

  return retVal;
}

/**
 * Saturate an input value between a upper and lower bound
 * 
 * @param paramValue The input value to be saturated
 * @param minValue The lower bound
 * @param maxValue The upper bound
 * @return The saturated input value.
 */
uint16_t saturateParamValue(uint16_t paramValue, uint16_t minValue, uint16_t maxValue) 
{
  uint16_t retValue = paramValue;

  if (paramValue > maxValue) 
  {
    retValue = maxValue;
  } else if (paramValue < minValue) 
  {
    retValue = minValue;
  }

  return retValue;
}


void saveEEPROM(StoredVar_type toSave) {
  g_pref.begin("stored_var", false);                      /* Open the "stored" namespace in read/write mode */
  g_pref.putBytes("user_param", &toSave, sizeof(toSave)); /* Put the value of the stored user_param */
  g_pref.end();                                           /* Close the namespace */
}
