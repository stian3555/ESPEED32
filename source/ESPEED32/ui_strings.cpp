#include "ui_strings.h"

/* Menu item names in different languages: [language][item] */
const char* MENU_NAMES[][11] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE", "PWM_F", "B_KNP", "R-Brems", "GRENSE", "INNSTILL", "STATS", "*BIL*"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "R-Brake", "LIMIT", "SETTINGS", "STATS", "*CAR*"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL", "PWM_F", "B_BTN", "R-Brake", "CHOKE1", "SETTINGS", "STATS", "*CAR*"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "PWM_F", "B_BTN", "R-Brake", "CHOKE", "SETTINGS", "STATS", "*CAR*"},
  /* ESP */ {"FRENO", "SENSI", "ANTIS", "CURVA", "PWM_F", "B_BTN", "R-Freno", "LIMITE", "AJUSTES", "STATS", "*AUTO*"},
  /* DEU */ {"BREMSE", "SENSI", "ANTIS", "KURVE", "PWM_F", "B_BTN", "R-Brems", "LIMIT", "SETUP", "STATS", "*AUTO*"},
  /* ITA */ {"FRENO", "SENSI", "ANTIS", "CURVA", "PWM_F", "B_BTN", "R-Freno", "LIMITE", "SETUP", "STATS", "*AUTO*"}
};

/* Settings menu item names: [language][item] */
const char* SETTINGS_MENU_NAMES[][13] = {
  /* NOR */ {"STROM", "SKJERM", "LYD", "EKST POT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "NULLSTILL", "TEST", "INFO", "TILBAKE"},
  /* ENG */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* CS  */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* ACD */ {"POWER", "DISPLAY", "SOUND", "EXT POT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "ABOUT", "BACK"},
  /* ESP */ {"ENERGIA", "PANTALLA", "SONIDO", "POT EXT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "ACERCA", "ATRAS"},
  /* DEU */ {"STROM", "ANZEIGE", "TON", "EXT POT", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "INFO", "ZURUCK"},
  /* ITA */ {"POTENZA", "SCHERMO", "SUONO", "POT EST", "STATS", "A STEP", "ENC INV", "WIFI", "USB INFO", "RESET", "TEST", "INFO", "INDIETRO"}
};

/* Power submenu item names: [language][item] */
const char* POWER_MENU_NAMES[][5] = {
  /* NOR */ {"SKJSP", "DVALE", "DYP SOV", "OPPSTART", "TILBAKE"},
  /* ENG */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"},
  /* CS  */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"},
  /* ACD */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "BACK"},
  /* ESP */ {"SCRSV", "REPOSO", "SUSP PROF", "ARRANQUE", "ATRAS"},
  /* DEU */ {"SCRSV", "SCHLAF", "TIEF SCHL", "START", "ZURUCK"},
  /* ITA */ {"SCRSV", "RIPOSO", "SONNO PROF", "AVVIO", "INDIETRO"}
};

/* Display submenu item names: [language][item] */
const char* DISPLAY_MENU_NAMES[][6] = {
  /* NOR */ {"RACEMODUS", "SPRAK", "STYL", "SKRIFTSTRL", "STATUSLINJE", "TILBAKE"},
  /* ENG */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"},
  /* CS  */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"},
  /* ACD */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "STATUS BAR", "BACK"},
  /* ESP */ {"MODO RACE", "IDIOMA", "ESTILO", "TAM TEXTO", "BARRA EST", "ATRAS"},
  /* DEU */ {"RACE MODUS", "SPRACHE", "STIL", "SCHRIFT", "STATUSLEISTE", "ZURUCK"},
  /* ITA */ {"MODO GARA", "LINGUA", "STILE", "DIM TESTO", "BARRA STATO", "INDIETRO"}
};

/* Race mode parameter labels: [language][param] */
const char* RACE_LABELS[][4] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE"},
  /* ESP */ {"FRENO", "SENSI", "ANTIS", "CURVA"},
  /* DEU */ {"BREMSE", "SENSI", "ANTIS", "KURVE"},
  /* ITA */ {"FRENO", "SENSI", "ANTIS", "CURVA"}
};

/* Car menu option labels: [language][option] */
const char* CAR_MENU_OPTIONS[][5] = {
  /* NOR */ {"VELG", "NAVNGI", "RACESWP", "KOPIER", "NULLSTILL"},
  /* ENG */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* CS  */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* ACD */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* ESP */ {"ELEGIR", "RENOMB", "RACESWP", "COPIAR", "RESET"},
  /* DEU */ {"WAHL", "NAME", "RACESWP", "KOPIER", "RESET"},
  /* ITA */ {"SCEGLI", "RINOM", "RACESWP", "COPIA", "RESET"}
};

/* View mode value labels: [language][mode] */
const char* VIEW_MODE_LABELS[][3] = {
  /* NOR */ {"AV", "FULL", "ENKEL"},
  /* ENG */ {"OFF", "FULL", "SIMPLE"},
  /* CS  */ {"OFF", "FULL", "SIMPLE"},
  /* ACD */ {"OFF", "FULL", "SIMPLE"},
  /* ESP */ {"OFF", "TOTAL", "SIMPLE"},
  /* DEU */ {"AUS", "VOLL", "EINF"},
  /* ITA */ {"OFF", "PIENO", "SEMPL"}
};

/* Sound submenu item names: [language][item] - BOOT, RACE, BACK */
const char* SOUND_MENU_NAMES[][3] = {
  /* NOR */ {"OPPSTART", "RACEMODE", "TILBAKE"},
  /* ENG */ {"BOOT", "RACE", "BACK"},
  /* CS  */ {"BOOT", "RACE", "BACK"},
  /* ACD */ {"BOOT", "RACE", "BACK"},
  /* ESP */ {"INICIO", "CARRERA", "ATRAS"},
  /* DEU */ {"START", "RENNEN", "ZURUCK"},
  /* ITA */ {"AVVIO", "GARA", "INDIETRO"}
};

/* ON/OFF labels: [language][state] */
const char* ON_OFF_LABELS[][2] = {
  /* NOR */ {"AV", "PA"},
  /* ENG */ {"OFF", "ON"},
  /* CS  */ {"OFF", "ON"},
  /* ACD */ {"OFF", "ON"},
  /* ESP */ {"OFF", "ON"},
  /* DEU */ {"AUS", "EIN"},
  /* ITA */ {"OFF", "ON"}
};

/* Release-brake mode labels: [language][mode] */
const char* RELEASE_BRAKE_MODE_LABELS[][3] = {
  /* NOR */ {"AV", "QCK", "DRG"},
  /* ENG */ {"OFF", "QCK", "DRG"},
  /* CS  */ {"OFF", "QCK", "DRG"},
  /* ACD */ {"OFF", "QCK", "DRG"},
  /* ESP */ {"OFF", "QCK", "DRG"},
  /* DEU */ {"AUS", "QCK", "DRG"},
  /* ITA */ {"OFF", "QCK", "DRG"}
};

/* Language labels: [language_code] */
const char* LANG_LABELS[] = {"NOR", "ENG", "CS", "ACD", "ESP", "DEU", "ITA"};

/* Text case style labels: [language][case_style] */
const char* TEXT_CASE_LABELS[][2] = {
  /* NOR */ {"UPPER", "Pascal"},
  /* ENG */ {"UPPER", "Pascal"},
  /* CS  */ {"UPPER", "Pascal"},
  /* ACD */ {"UPPER", "Pascal"},
  /* ESP */ {"MAYUS", "Pascal"},
  /* DEU */ {"GROSS", "Pascal"},
  /* ITA */ {"MAIUS", "Pascal"}
};

/* Font size labels: [language][size] */
const char* FONT_SIZE_LABELS[][2] = {
  /* NOR */ {"STOR", "liten"},
  /* ENG */ {"LARGE", "small"},
  /* CS  */ {"LARGE", "small"},
  /* ACD */ {"LARGE", "small"},
  /* ESP */ {"GRAND", "peq."},
  /* DEU */ {"GROSS", "klein"},
  /* ITA */ {"GRAND", "picc."}
};

/* Menu item names - Pascal Case: [language][item] */
const char* MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve", "Pwm_F", "B_Knp", "R-Brems", "Grense", "Innstill", "Stats", "*Bil*"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "R-Brake", "Limit", "Settings", "Stats", "*Car*"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil", "Pwm_F", "B_Btn", "R-Brake", "Choke1", "Settings", "Stats", "*Car*"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve", "Pwm_F", "B_Btn", "R-Brake", "Choke", "Settings", "Stats", "*Car*"},
  /* ESP */ {"Freno", "Sensi", "Antis", "Curva", "Pwm_F", "B_Btn", "R-Freno", "Limite", "Ajustes", "Stats", "*Auto*"},
  /* DEU */ {"Bremse", "Sensi", "Antis", "Kurve", "Pwm_F", "B_Btn", "R-Brems", "Limit", "Setup", "Stats", "*Auto*"},
  /* ITA */ {"Freno", "Sensi", "Antis", "Curva", "Pwm_F", "B_Btn", "R-Freno", "Limite", "Setup", "Stats", "*Auto*"}
};

/* Settings menu item names - Pascal Case: [language][item] */
const char* SETTINGS_MENU_NAMES_PASCAL[][13] = {
  /* NOR */ {"Strom", "Skjerm", "Lyd", "Ekst pot", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Nullstill", "Test", "Info", "Tilbake"},
  /* ENG */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* CS  */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* ACD */ {"Power", "Display", "Sound", "Ext Pot", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "About", "Back"},
  /* ESP */ {"Energia", "Pantalla", "Sonido", "Pot Ext", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "Acerca", "Atras"},
  /* DEU */ {"Strom", "Anzeige", "Ton", "Ext Pot", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "Info", "Zuruck"},
  /* ITA */ {"Potenza", "Schermo", "Suono", "Pot est", "Stats", "A Step", "Enc Inv", "Wifi", "Usb info", "Reset", "Test", "Info", "Indietro"}
};

/* Power submenu item names - Pascal Case: [language][item] */
const char* POWER_MENU_NAMES_PASCAL[][5] = {
  /* NOR */ {"Skjsp", "Dvale", "Dyp Sov", "Oppstart", "Tilbake"},
  /* ENG */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"},
  /* CS  */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"},
  /* ACD */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Back"},
  /* ESP */ {"Scrsv", "Reposo", "Susp Prof", "Arranque", "Atras"},
  /* DEU */ {"Scrsv", "Schlaf", "Tief Schl", "Start", "Zuruck"},
  /* ITA */ {"Scrsv", "Riposo", "Sonno Prof", "Avvio", "Indietro"}
};

/* Display submenu item names - Pascal Case: [language][item] */
const char* DISPLAY_MENU_NAMES_PASCAL[][6] = {
  /* NOR */ {"Racemodus", "Sprak", "Styl", "Skriftstrl", "Statuslinje", "Tilbake"},
  /* ENG */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"},
  /* CS  */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"},
  /* ACD */ {"Race Mode", "Language", "Case", "Font Size", "Status bar", "Back"},
  /* ESP */ {"Modo Race", "Idioma", "Estilo", "Tam texto", "Barra est", "Atras"},
  /* DEU */ {"Race Modus", "Sprache", "Stil", "Schrift", "Statusleiste", "Zuruck"},
  /* ITA */ {"Modo Gara", "Lingua", "Stile", "Dim testo", "Barra stato", "Indietro"}
};

/* Race mode parameter labels - Pascal Case: [language][param] */
const char* RACE_LABELS_PASCAL[][4] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve"},
  /* ESP */ {"Freno", "Sensi", "Antis", "Curva"},
  /* DEU */ {"Bremse", "Sensi", "Antis", "Kurve"},
  /* ITA */ {"Freno", "Sensi", "Antis", "Curva"}
};

/* Car menu option labels - Pascal Case: [language][option] */
const char* CAR_MENU_OPTIONS_PASCAL[][5] = {
  /* NOR */ {"Velg", "Navngi", "Raceswp", "Kopier", "Nullstill"},
  /* ENG */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* CS  */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* ACD */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* ESP */ {"Elegir", "Renomb", "Raceswp", "Copiar", "Reset"},
  /* DEU */ {"Wahl", "Name", "Raceswp", "Kopier", "Reset"},
  /* ITA */ {"Scegli", "Rinom", "Raceswp", "Copia", "Reset"}
};

/* View mode value labels - Pascal Case: [language][mode] */
const char* VIEW_MODE_LABELS_PASCAL[][3] = {
  /* NOR */ {"Av", "Full", "Enkel"},
  /* ENG */ {"Off", "Full", "Simple"},
  /* CS  */ {"Off", "Full", "Simple"},
  /* ACD */ {"Off", "Full", "Simple"},
  /* ESP */ {"Off", "Total", "Simple"},
  /* DEU */ {"Aus", "Voll", "Einf"},
  /* ITA */ {"Off", "Pieno", "Sempl"}
};

/* Sound submenu item names - Pascal Case: [language][item] */
const char* SOUND_MENU_NAMES_PASCAL[][3] = {
  /* NOR */ {"Oppstart", "Racemode", "Tilbake"},
  /* ENG */ {"Boot", "Race", "Back"},
  /* CS  */ {"Boot", "Race", "Back"},
  /* ACD */ {"Boot", "Race", "Back"},
  /* ESP */ {"Inicio", "Carrera", "Atras"},
  /* DEU */ {"Start", "Rennen", "Zuruck"},
  /* ITA */ {"Avvio", "Gara", "Indietro"}
};

/* ON/OFF labels - Pascal Case: [language][state] */
const char* ON_OFF_LABELS_PASCAL[][2] = {
  /* NOR */ {"Av", "Pa"},
  /* ENG */ {"Off", "On"},
  /* CS  */ {"Off", "On"},
  /* ACD */ {"Off", "On"},
  /* ESP */ {"Off", "On"},
  /* DEU */ {"Aus", "Ein"},
  /* ITA */ {"Off", "On"}
};

/* Release-brake mode labels - Pascal Case: [language][mode] */
const char* RELEASE_BRAKE_MODE_LABELS_PASCAL[][3] = {
  /* NOR */ {"Av", "Qck", "Drg"},
  /* ENG */ {"Off", "Qck", "Drg"},
  /* CS  */ {"Off", "Qck", "Drg"},
  /* ACD */ {"Off", "Qck", "Drg"},
  /* ESP */ {"Off", "Qck", "Drg"},
  /* DEU */ {"Aus", "Qck", "Drg"},
  /* ITA */ {"Off", "Qck", "Drg"}
};

/* BACK button labels - UPPER CASE: [language] */
const char* BACK_LABELS[] = {
  /* NOR */ "TILBAKE",
  /* ENG */ "BACK",
  /* CS  */ "BACK",
  /* ACD */ "BACK",
  /* ESP */ "ATRAS",
  /* DEU */ "ZURUCK",
  /* ITA */ "INDIETRO"
};

/* BACK button labels - Pascal Case: [language] */
const char* BACK_LABELS_PASCAL[] = {
  /* NOR */ "Tilbake",
  /* ENG */ "Back",
  /* CS  */ "Back",
  /* ACD */ "Back",
  /* ESP */ "Atras",
  /* DEU */ "Zuruck",
  /* ITA */ "Indietro"
};

/* UI strings for car selection/copy/rename/reset screens: [language] */
const char* STR_SELECT_CAR[] = {
  "-VELG BIL-", "-SELECT THE CAR-", "-SELECT THE CAR-", "-SELECT THE CAR-",
  "-ELEGIR AUTO-", "-AUTO WAEHLEN-", "-SCEGLI AUTO-"
};
const char* STR_COPY_FROM[] = {
  "-KOPIER FRA:-", "-COPY FROM:-", "-COPY FROM:-", "-COPY FROM:-",
  "-COPIAR DE:-", "-KOPIER VON:-", "-COPIA DA:-"
};
const char* STR_COPY_TO[] = {
  "-KOPIER TIL:-", "-COPY TO:-", "-COPY TO:-", "-COPY TO:-",
  "-COPIAR A:-", "-KOPIER ZU:-", "-COPIA A:-"
};
const char* STR_ALL[] = {"ALLE", "ALL", "ALL", "ALL", "TODO", "ALLE", "TUTT"};
const char* STR_COPIED_ALL[] = {
  "KOPIERT!", "COPIED ALL", "COPIED ALL", "COPIED ALL",
  "TODO COP", "ALLE KOP", "TUTTE OK"
};
const char* STR_COPIED[] = {"KOPIERT!", "COPIED!", "COPIED!", "COPIED!", "COPIADO", "KOPIERT", "COPIATO"};
const char* STR_RENAME_CAR[] = {
  "-GI NYTT NAVN-", "-RENAME THE CAR-", "-RENAME THE CAR-", "-RENAME THE CAR-",
  "-RENOMB AUTO-", "-AUTO BENENN-", "-RINOM AUTO-"
};
const char* STR_CONFIRM[] = {
  "-TRYKK OK FOR GODTA-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-",
  "-OK CONFIRMAR-", "-OK BESTAETIG-", "-OK CONFERMA-"
};
const char* STR_LIMITER[] = {
  " - BEGRENS - ", " - LIMITER - ", " - CHOKE1 - ", " - CHOKE - ",
  " - LIMITE - ", " - LIMIT - ", " - LIMITE - "
};
const char* STR_CALIBRATION[] = {
  "KALIBRERING", "CALIBRATION", "CALIBRATION", "CALIBRATION",
  "CALIBRACION", "KALIBRATION", "CALIBRAZIONE"
};
const char* STR_PRESS_RELEASE[] = {
  "trykk/slipp gass", "press/release trg", "press/release trg", "press/release trg",
  "pulsa/suelta trg", "drueck/los trg", "premi/molla trg"
};
const char* STR_PUSH_DONE[] = {
  " trykk nar ferdig ", " press when done ", " press when done ", " press when done ",
  " pulsa al final  ", " druck bei ende ", " premi a fine   "
};
