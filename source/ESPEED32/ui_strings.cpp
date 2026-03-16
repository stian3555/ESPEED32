#include "ui_strings.h"

/* Menu item names in different languages: [language][item] */
const char* MENU_NAMES[][11] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE", "PWM_F", "B_KNP", "H-Brems", "GRENSE", "INNSTILL", "STATS", "*BIL*"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "Q-Brake", "LIMIT", "SETTINGS", "STATS", "*CAR*"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL", "PWM_F", "B_BTN", "Q-Brake", "CHOKE1", "SETTINGS", "STATS", "*CAR*"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "Q-Brake", "CHOKE", "SETTINGS", "STATS", "*CAR*"}
};

/* Settings menu item names: [language][item] */
const char* SETTINGS_MENU_NAMES[][11] = {
  /* NOR */ {"STROM", "SKJERM", "LYD", "EKST POT", "STATS", "WIFI", "USB INFO", "NULLSTILL", "TEST", "INFO", "TILBAKE"},
  /* ENG */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* CS  */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* ACD */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"}
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

/* Sound submenu item names: [language][item] - BOOT, RACE, BACK */
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

/* Menu item names - Pascal Case: [language][item] */
const char* MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve", "Pwm_F", "B_Knp", "H-Brems", "Grense", "Innstill", "Stats", "*Bil*"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "Q-Brake", "Limit", "Settings", "Stats", "*Car*"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil", "Pwm_F", "B_Btn", "Q-Brake", "Choke1", "Settings", "Stats", "*Car*"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "Q-Brake", "Choke", "Settings", "Stats", "*Car*"}
};

/* Settings menu item names - Pascal Case: [language][item] */
const char* SETTINGS_MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Strom", "Skjerm", "Lyd", "Ekst pot", "Stats", "Wifi", "Usb info", "Nullstill", "Test", "Info", "Tilbake"},
  /* ENG */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* CS  */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* ACD */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "Wifi", "Usb info", "Reset", "Test", "About", "Back"}
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
const char* STR_PRESS_RELEASE[] = { "trykk/slipp gass", "press/releas throttle", "press/releas throttle", "press/releas throttle" };
const char* STR_PUSH_DONE[] = { " trykk nar ferdig ", " push when done ", " push when done ", " push when done " };
