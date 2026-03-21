#include "ui_strings.h"

/* Menu item names in different languages: [language][item] */
const char* MENU_NAMES[][11] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE", "FADE", "PWM_F", "BREMS+", "GRENSE", "INNSTILL", "STATS", "*BIL*"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "FADE", "PWM_F", "BRAKE+", "LIMIT", "SETTINGS", "STATS", "*CAR*"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL", "FADE", "PWM_F", "BRAKE+", "CHOKE1", "SETTINGS", "STATS", "*CAR*"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE", "FADE", "PWM_F", "BRAKE+", "CHOKE", "SETTINGS", "STATS", "*CAR*"},
  /* ESP */ {"FRENO", "SENSI", "ANTIS", "CURVA", "FADE", "PWM_F", "FRENO+", "LIMITE", "AJUSTES", "STATS", "*AUTO*"},
  /* DEU */ {"BREMSE", "SENSI", "ANTIS", "KURVE", "FADE", "PWM_F", "BREMSE+", "LIMIT", "SETUP", "STATS", "*AUTO*"},
  /* ITA */ {"FRENO", "SENSI", "ANTIS", "CURVA", "FADE", "PWM_F", "FRENO+", "LIMITE", "SETUP", "STATS", "*AUTO*"},
  /* NLD */ {"REM", "SENSI", "ANTIS", "CURVE", "FADE", "PWM_F", "REM+", "LIMIET", "INSTELL", "STATS", "*AUTO*"}
};

/* Settings menu item names: [language][item] */
const char* SETTINGS_MENU_NAMES[][11] = {
  /* NOR */ {"STROM", "SKJERM", "LYD", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "NULLSTILL", "INFO", "TILBAKE"},
  /* ENG */ {"POWER", "DISPLAY", "SOUND", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "ABOUT", "BACK"},
  /* CS  */ {"POWER", "DISPLAY", "SOUND", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "ABOUT", "BACK"},
  /* ACD */ {"POWER", "DISPLAY", "SOUND", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "ABOUT", "BACK"},
  /* ESP */ {"ENERGIA", "PANTALLA", "SONIDO", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "ACERCA", "ATRAS"},
  /* DEU */ {"STROM", "ANZEIGE", "TON", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "INFO", "ZURUCK"},
  /* ITA */ {"POTENZA", "SCHERMO", "SUONO", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "INFO", "INDIETRO"},
  /* NLD */ {"STROOM", "DISPLAY", "GELUID", "HARDWARE", "STATS", "WIFI", "LOGGING", "USB INFO", "RESET", "INFO", "TERUG"}
};

/* Power submenu item names: [language][item] */
const char* POWER_MENU_NAMES[][6] = {
  /* NOR */ {"SKJSP", "DVALE", "DYP SOV", "OPPSTART", "VIN KAL.", "TILBAKE"},
  /* ENG */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "VIN CAL.", "BACK"},
  /* CS  */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "VIN CAL.", "BACK"},
  /* ACD */ {"SCRSV", "SLEEP", "DEEP SLEEP", "STARTUP", "VIN CAL.", "BACK"},
  /* ESP */ {"SCRSV", "REPOSO", "SUSP PROF", "ARRANQUE", "VIN CAL.", "ATRAS"},
  /* DEU */ {"SCRSV", "SCHLAF", "TIEF SCHL", "START", "VIN KAL.", "ZURUCK"},
  /* ITA */ {"SCRSV", "RIPOSO", "SONNO PROF", "AVVIO", "VIN CAL.", "INDIETRO"},
  /* NLD */ {"SCRSV", "SLAPEN", "DIEP SLP", "START", "VIN KAL.", "TERUG"}
};

/* Display submenu item names: [language][item] */
const char* DISPLAY_MENU_NAMES[][7] = {
  /* NOR */ {"RACEMODUS", "SPRAK", "STYL", "SKRIFTSTRL", "ANTIS.STEP", "STATUSLINJE", "TILBAKE"},
  /* ENG */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "ANTIS.STEP", "STATUS BAR", "BACK"},
  /* CS  */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "ANTIS.STEP", "STATUS BAR", "BACK"},
  /* ACD */ {"RACE MODE", "LANGUAGE", "CASE", "FONT SIZE", "ANTIS.STEP", "STATUS BAR", "BACK"},
  /* ESP */ {"MODO RACE", "IDIOMA", "ESTILO", "TAM TEXTO", "ANTIS.STEP", "BARRA EST", "ATRAS"},
  /* DEU */ {"RACE MODUS", "SPRACHE", "STIL", "SCHRIFT", "ANTIS.STEP", "STATUSLEISTE", "ZURUCK"},
  /* ITA */ {"MODO GARA", "LINGUA", "STILE", "DIM TESTO", "ANTIS.STEP", "BARRA STATO", "INDIETRO"},
  /* NLD */ {"RACE MODUS", "TAAL", "STIJL", "TEKSTGRT", "ANTIS.STEP", "STATUSBALK", "TERUG"}
};

/* Race mode parameter labels: [language][param] */
const char* RACE_LABELS[][4] = {
  /* NOR */ {"BREMS", "SENSI", "ANTIS", "KURVE"},
  /* ENG */ {"BRAKE", "SENSI", "ANTIS", "CURVE"},
  /* CS  */ {"BRAKE", "ATTACK", "CHOKE2", "PROFIL"},
  /* ACD */ {"BRAKE", "SENSI", "ANTIS", "CURVE"},
  /* ESP */ {"FRENO", "SENSI", "ANTIS", "CURVA"},
  /* DEU */ {"BREMSE", "SENSI", "ANTIS", "KURVE"},
  /* ITA */ {"FRENO", "SENSI", "ANTIS", "CURVA"},
  /* NLD */ {"REM", "SENSI", "ANTIS", "CURVE"}
};

/* Car menu option labels: [language][option] */
const char* CAR_MENU_OPTIONS[][5] = {
  /* NOR */ {"VELG", "NAVNGI", "RACESWP", "KOPIER", "NULLSTILL"},
  /* ENG */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* CS  */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* ACD */ {"SELECT", "RENAME", "RACESWP", "COPY", "RESET"},
  /* ESP */ {"ELEGIR", "RENOMB", "RACESWP", "COPIAR", "RESET"},
  /* DEU */ {"WAHL", "NAME", "RACESWP", "KOPIER", "RESET"},
  /* ITA */ {"SCEGLI", "RINOM", "RACESWP", "COPIA", "RESET"},
  /* NLD */ {"KIES", "HERNOEM", "RACESWP", "KOPIE", "RESET"}
};

/* View mode value labels: [language][mode] */
const char* VIEW_MODE_LABELS[][3] = {
  /* NOR */ {"AV", "FULL", "ENKEL"},
  /* ENG */ {"OFF", "FULL", "SIMPLE"},
  /* CS  */ {"OFF", "FULL", "SIMPLE"},
  /* ACD */ {"OFF", "FULL", "SIMPLE"},
  /* ESP */ {"OFF", "TOTAL", "SIMPLE"},
  /* DEU */ {"AUS", "VOLL", "EINF"},
  /* ITA */ {"OFF", "PIENO", "SEMPL"},
  /* NLD */ {"UIT", "VOL", "EENV"}
};

/* Sound submenu item names: [language][item] - BOOT, RACE, BACK */
const char* SOUND_MENU_NAMES[][3] = {
  /* NOR */ {"OPPSTART", "RACEMODE", "TILBAKE"},
  /* ENG */ {"BOOT", "RACE", "BACK"},
  /* CS  */ {"BOOT", "RACE", "BACK"},
  /* ACD */ {"BOOT", "RACE", "BACK"},
  /* ESP */ {"INICIO", "CARRERA", "ATRAS"},
  /* DEU */ {"START", "RENNEN", "ZURUCK"},
  /* ITA */ {"AVVIO", "GARA", "INDIETRO"},
  /* NLD */ {"START", "RACE", "TERUG"}
};

/* ON/OFF labels: [language][state] */
const char* ON_OFF_LABELS[][2] = {
  /* NOR */ {"AV", "PA"},
  /* ENG */ {"OFF", "ON"},
  /* CS  */ {"OFF", "ON"},
  /* ACD */ {"OFF", "ON"},
  /* ESP */ {"OFF", "ON"},
  /* DEU */ {"AUS", "EIN"},
  /* ITA */ {"OFF", "ON"},
  /* NLD */ {"UIT", "AAN"}
};

/* Release-brake mode labels: [language][mode] */
const char* RELEASE_BRAKE_MODE_LABELS[][3] = {
  /* NOR */ {"AV", "QCK", "DRG"},
  /* ENG */ {"OFF", "QCK", "DRG"},
  /* CS  */ {"OFF", "QCK", "DRG"},
  /* ACD */ {"OFF", "QCK", "DRG"},
  /* ESP */ {"OFF", "QCK", "DRG"},
  /* DEU */ {"AUS", "QCK", "DRG"},
  /* ITA */ {"OFF", "QCK", "DRG"},
  /* NLD */ {"UIT", "QCK", "DRG"}
};

/* Language labels: [language_code] */
const char* LANG_LABELS[] = {"NOR", "ENG", "CS", "ACD", "ESP", "DEU", "ITA", "NLD"};

/* Text case style labels: [language][case_style] */
const char* TEXT_CASE_LABELS[][2] = {
  /* NOR */ {"UPPER", "Pascal"},
  /* ENG */ {"UPPER", "Pascal"},
  /* CS  */ {"UPPER", "Pascal"},
  /* ACD */ {"UPPER", "Pascal"},
  /* ESP */ {"MAYUS", "Pascal"},
  /* DEU */ {"GROSS", "Pascal"},
  /* ITA */ {"MAIUS", "Pascal"},
  /* NLD */ {"HOOFD", "Pascal"}
};

/* Font size labels: [language][size] */
const char* FONT_SIZE_LABELS[][2] = {
  /* NOR */ {"STOR", "liten"},
  /* ENG */ {"LARGE", "small"},
  /* CS  */ {"LARGE", "small"},
  /* ACD */ {"LARGE", "small"},
  /* ESP */ {"GRAND", "peq."},
  /* DEU */ {"GROSS", "klein"},
  /* ITA */ {"GRAND", "picc."},
  /* NLD */ {"GROOT", "klein"}
};

/* Menu item names - Pascal Case: [language][item] */
const char* MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve", "Fade", "Pwm_F", "Brems+", "Grense", "Innstill", "Stats", "*Bil*"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve", "Fade", "Pwm_F", "Brake+", "Limit", "Settings", "Stats", "*Car*"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil", "Fade", "Pwm_F", "Brake+", "Choke1", "Settings", "Stats", "*Car*"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve", "Fade", "Pwm_F", "Brake+", "Choke", "Settings", "Stats", "*Car*"},
  /* ESP */ {"Freno", "Sensi", "Antis", "Curva", "Fade", "Pwm_F", "Freno+", "Limite", "Ajustes", "Stats", "*Auto*"},
  /* DEU */ {"Bremse", "Sensi", "Antis", "Kurve", "Fade", "Pwm_F", "Bremse+", "Limit", "Setup", "Stats", "*Auto*"},
  /* ITA */ {"Freno", "Sensi", "Antis", "Curva", "Fade", "Pwm_F", "Freno+", "Limite", "Setup", "Stats", "*Auto*"},
  /* NLD */ {"Rem", "Sensi", "Antis", "Curve", "Fade", "Pwm_F", "Rem+", "Limiet", "Instell", "Stats", "*Auto*"}
};

/* Settings menu item names - Pascal Case: [language][item] */
const char* SETTINGS_MENU_NAMES_PASCAL[][11] = {
  /* NOR */ {"Strom", "Skjerm", "Lyd", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Nullstill", "Info", "Tilbake"},
  /* ENG */ {"Power", "Display", "Sound", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "About", "Back"},
  /* CS  */ {"Power", "Display", "Sound", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "About", "Back"},
  /* ACD */ {"Power", "Display", "Sound", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "About", "Back"},
  /* ESP */ {"Energia", "Pantalla", "Sonido", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "Acerca", "Atras"},
  /* DEU */ {"Strom", "Anzeige", "Ton", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "Info", "Zuruck"},
  /* ITA */ {"Potenza", "Schermo", "Suono", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "Info", "Indietro"},
  /* NLD */ {"Stroom", "Display", "Geluid", "Hardware", "Stats", "Wifi", "Logging", "Usb info", "Reset", "Info", "Terug"}
};

/* Power submenu item names - Pascal Case: [language][item] */
const char* POWER_MENU_NAMES_PASCAL[][6] = {
  /* NOR */ {"Skjsp", "Dvale", "Dyp Sov", "Oppstart", "Vin kal.", "Tilbake"},
  /* ENG */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Vin Cal.", "Back"},
  /* CS  */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Vin Cal.", "Back"},
  /* ACD */ {"Scrsv", "Sleep", "Deep Sleep", "Startup", "Vin Cal.", "Back"},
  /* ESP */ {"Scrsv", "Reposo", "Susp Prof", "Arranque", "Vin Cal.", "Atras"},
  /* DEU */ {"Scrsv", "Schlaf", "Tief Schl", "Start", "Vin Kal.", "Zuruck"},
  /* ITA */ {"Scrsv", "Riposo", "Sonno Prof", "Avvio", "Vin Cal.", "Indietro"},
  /* NLD */ {"Scrsv", "Slapen", "Diep Slp", "Start", "Vin kal.", "Terug"}
};

/* Display submenu item names - Pascal Case: [language][item] */
const char* DISPLAY_MENU_NAMES_PASCAL[][7] = {
  /* NOR */ {"Racemodus", "Sprak", "Styl", "Skriftstrl", "Antis.Step", "Statuslinje", "Tilbake"},
  /* ENG */ {"Race Mode", "Language", "Case", "Font Size", "Antis.Step", "Status bar", "Back"},
  /* CS  */ {"Race Mode", "Language", "Case", "Font Size", "Antis.Step", "Status bar", "Back"},
  /* ACD */ {"Race Mode", "Language", "Case", "Font Size", "Antis.Step", "Status bar", "Back"},
  /* ESP */ {"Modo Race", "Idioma", "Estilo", "Tam texto", "Antis.Step", "Barra est", "Atras"},
  /* DEU */ {"Race Modus", "Sprache", "Stil", "Schrift", "Antis.Step", "Statusleiste", "Zuruck"},
  /* ITA */ {"Modo Gara", "Lingua", "Stile", "Dim testo", "Antis.Step", "Barra stato", "Indietro"},
  /* NLD */ {"Race Modus", "Taal", "Stijl", "Tekstgrt", "Antis.Step", "Statusbalk", "Terug"}
};

/* Race mode parameter labels - Pascal Case: [language][param] */
const char* RACE_LABELS_PASCAL[][4] = {
  /* NOR */ {"Brems", "Sensi", "Antis", "Kurve"},
  /* ENG */ {"Brake", "Sensi", "Antis", "Curve"},
  /* CS  */ {"Brake", "Attack", "Choke2", "Profil"},
  /* ACD */ {"Brake", "Sensi", "Antis", "Curve"},
  /* ESP */ {"Freno", "Sensi", "Antis", "Curva"},
  /* DEU */ {"Bremse", "Sensi", "Antis", "Kurve"},
  /* ITA */ {"Freno", "Sensi", "Antis", "Curva"},
  /* NLD */ {"Rem", "Sensi", "Antis", "Curve"}
};

/* Car menu option labels - Pascal Case: [language][option] */
const char* CAR_MENU_OPTIONS_PASCAL[][5] = {
  /* NOR */ {"Velg", "Navngi", "Raceswp", "Kopier", "Nullstill"},
  /* ENG */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* CS  */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* ACD */ {"Select", "Rename", "Raceswp", "Copy", "Reset"},
  /* ESP */ {"Elegir", "Renomb", "Raceswp", "Copiar", "Reset"},
  /* DEU */ {"Wahl", "Name", "Raceswp", "Kopier", "Reset"},
  /* ITA */ {"Scegli", "Rinom", "Raceswp", "Copia", "Reset"},
  /* NLD */ {"Kies", "Hernoem", "Raceswp", "Kopie", "Reset"}
};

/* View mode value labels - Pascal Case: [language][mode] */
const char* VIEW_MODE_LABELS_PASCAL[][3] = {
  /* NOR */ {"Av", "Full", "Enkel"},
  /* ENG */ {"Off", "Full", "Simple"},
  /* CS  */ {"Off", "Full", "Simple"},
  /* ACD */ {"Off", "Full", "Simple"},
  /* ESP */ {"Off", "Total", "Simple"},
  /* DEU */ {"Aus", "Voll", "Einf"},
  /* ITA */ {"Off", "Pieno", "Sempl"},
  /* NLD */ {"Uit", "Vol", "Eenv"}
};

/* Sound submenu item names - Pascal Case: [language][item] */
const char* SOUND_MENU_NAMES_PASCAL[][3] = {
  /* NOR */ {"Oppstart", "Racemode", "Tilbake"},
  /* ENG */ {"Boot", "Race", "Back"},
  /* CS  */ {"Boot", "Race", "Back"},
  /* ACD */ {"Boot", "Race", "Back"},
  /* ESP */ {"Inicio", "Carrera", "Atras"},
  /* DEU */ {"Start", "Rennen", "Zuruck"},
  /* ITA */ {"Avvio", "Gara", "Indietro"},
  /* NLD */ {"Start", "Race", "Terug"}
};

/* ON/OFF labels - Pascal Case: [language][state] */
const char* ON_OFF_LABELS_PASCAL[][2] = {
  /* NOR */ {"Av", "Pa"},
  /* ENG */ {"Off", "On"},
  /* CS  */ {"Off", "On"},
  /* ACD */ {"Off", "On"},
  /* ESP */ {"Off", "On"},
  /* DEU */ {"Aus", "Ein"},
  /* ITA */ {"Off", "On"},
  /* NLD */ {"Uit", "Aan"}
};

/* Release-brake mode labels - Pascal Case: [language][mode] */
const char* RELEASE_BRAKE_MODE_LABELS_PASCAL[][3] = {
  /* NOR */ {"Av", "Qck", "Drg"},
  /* ENG */ {"Off", "Qck", "Drg"},
  /* CS  */ {"Off", "Qck", "Drg"},
  /* ACD */ {"Off", "Qck", "Drg"},
  /* ESP */ {"Off", "Qck", "Drg"},
  /* DEU */ {"Aus", "Qck", "Drg"},
  /* ITA */ {"Off", "Qck", "Drg"},
  /* NLD */ {"Uit", "Qck", "Drg"}
};

/* BACK button labels - UPPER CASE: [language] */
const char* BACK_LABELS[] = {
  /* NOR */ "TILBAKE",
  /* ENG */ "BACK",
  /* CS  */ "BACK",
  /* ACD */ "BACK",
  /* ESP */ "ATRAS",
  /* DEU */ "ZURUCK",
  /* ITA */ "INDIETRO",
  /* NLD */ "TERUG"
};

/* BACK button labels - Pascal Case: [language] */
const char* BACK_LABELS_PASCAL[] = {
  /* NOR */ "Tilbake",
  /* ENG */ "Back",
  /* CS  */ "Back",
  /* ACD */ "Back",
  /* ESP */ "Atras",
  /* DEU */ "Zuruck",
  /* ITA */ "Indietro",
  /* NLD */ "Terug"
};

/* UI strings for car selection/copy/rename/reset screens: [language] */
const char* STR_SELECT_CAR[] = {
  "-VELG BIL-", "-SELECT THE CAR-", "-SELECT THE CAR-", "-SELECT THE CAR-",
  "-ELEGIR AUTO-", "-AUTO WAEHLEN-", "-SCEGLI AUTO-", "-KIES AUTO-"
};
const char* STR_COPY_FROM[] = {
  "-KOPIER FRA:-", "-COPY FROM:-", "-COPY FROM:-", "-COPY FROM:-",
  "-COPIAR DE:-", "-KOPIER VON:-", "-COPIA DA:-", "-KOPIE VAN:-"
};
const char* STR_COPY_TO[] = {
  "-KOPIER TIL:-", "-COPY TO:-", "-COPY TO:-", "-COPY TO:-",
  "-COPIAR A:-", "-KOPIER ZU:-", "-COPIA A:-", "-KOPIE NAAR-"
};
const char* STR_ALL[] = {"ALLE", "ALL", "ALL", "ALL", "TODO", "ALLE", "TUTT", "ALLE"};
const char* STR_COPIED_ALL[] = {
  "KOPIERT!", "COPIED ALL", "COPIED ALL", "COPIED ALL",
  "TODO COP", "ALLE KOP", "TUTTE OK", "ALLES OK"
};
const char* STR_COPIED[] = {"KOPIERT!", "COPIED!", "COPIED!", "COPIED!", "COPIADO", "KOPIERT", "COPIATO", "KOPIE OK"};
const char* STR_RENAME_CAR[] = {
  "-GI NYTT NAVN-", "-RENAME THE CAR-", "-RENAME THE CAR-", "-RENAME THE CAR-",
  "-RENOMB AUTO-", "-AUTO BENENN-", "-RINOM AUTO-", "-HERNOEM AUTO-"
};
const char* STR_CONFIRM[] = {
  "-TRYKK OK FOR GODTA-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-", "-CLICK OK TO CONFIRM-",
  "-OK CONFIRMAR-", "-OK BESTAETIG-", "-OK CONFERMA-", "-DRUK OK BEVEST-"
};
const char* STR_LIMITER[] = {
  " - BEGRENS - ", " - LIMITER - ", " - CHOKE1 - ", " - CHOKE - ",
  " - LIMITE - ", " - LIMIT - ", " - LIMITE - ", " - LIMIET - "
};
const char* STR_CALIBRATION[] = {
  "KALIBRERING", "CALIBRATION", "CALIBRATION", "CALIBRATION",
  "CALIBRACION", "KALIBRATION", "CALIBRAZIONE", "KALIBRATIE"
};
const char* STR_PRESS_RELEASE[] = {
  "trykk/slipp gass", "press/release trg", "press/release trg", "press/release trg",
  "pulsa/suelta trg", "drueck/los trg", "premi/molla trg", "druk/los trg"
};
const char* STR_PUSH_DONE[] = {
  " trykk nar ferdig ", " press when done ", " press when done ", " press when done ",
  " pulsa al final  ", " druck bei ende ", " premi a fine   ", " druk als klaar "
};
