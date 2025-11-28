/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include "slot_ESC.h"

/*********************************************************************************************************************/
/*                                                   Version Control                                                 */
/*********************************************************************************************************************/
#define SW_MAJOR_VERSION 2
#define SW_MINOR_VERSION 6
#define STORED_VAR_VERSION 4  /* Stored variable version - increment when stored structure changes */

/* Last modified: 17/10/2024 */
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

/* Stored Variables (EEPROM/Preferences) */
StoredVar_type g_storedVar;

/* ESC Runtime Variables */
ESC_type g_escVar {
  .outputSpeed_pct = 0,
  .trigger_raw = 0,
  .trigger_norm = 0,
  .encoderPos = 1,
  .Vin_mV = 0,
  .dualCurve = false
};

/* Menu Structures */
Menu_type g_mainMenu {
  .lines = 3
};

Menu_type g_carMenu {
  .lines = 3
};

/* Preferences (NVM storage) */
Preferences g_pref;

/* UI Timing */
static uint32_t g_lastEncoderInteraction = 0;  /* Timestamp of last encoder interaction for display power saving */

/*********************************************************************************************************************/
/*                                                Function Prototypes                                                */
/*********************************************************************************************************************/
void IRAM_ATTR readEncoderISR();

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

    /* Read input voltage and motor current */
    g_escVar.Vin_mV = HAL_ReadVoltageDivider(AN_VIN_DIV, RVIFBL, RVIFBH);
    g_escVar.motorCurrent_mA = HAL_ReadMotorCurrent();

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

            /* If button is pressed at startup, go to CALIBRATION state */
            if (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED) 
            {
              g_currState = CALIBRATION;      /* Go to CALIBRATION state */
              /* Reset Min and Max to the opposite side, in order to have effective calibration */
              g_storedVar.minTrigger_raw = MAX_INT16;
              g_storedVar.maxTrigger_raw = MIN_INT16;
              calibSound();             /* Play calibration sound */
              initDisplayAndEncoder();  /* init and clear OLED and Encoder */

              /* Wait until button is released, then go to CALIBRATION state */
              while (digitalRead(ENCODER_BUTTON_PIN) == BUTTON_PRESSED)
              {
                showScreenPreCalibration();
              }
              
              obdFill(&g_obd, OBD_WHITE, 1); /* Clear OLED */
            }
            else  /* If button is NOT pressed at startup, go to RUNNING state */
            {
              g_currState = WELCOME;                    /* Go to WELCOME state */
              g_carSel = g_storedVar.selectedCarNumber; /* now it is safe to address the proper car */
              initDisplayAndEncoder();  /* init and clear OLED and Encoder */
              onSound();                /* Play ON sound */
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
        calibSound();                   /* Play calibration sound */
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
          offSound();
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

        showScreenWelcome();    /* Show welcome screen */
        delay(1500);            /* Wait one second */
        g_currState = RUNNING;  /* Go to RUNNING state */
        break;


      case RUNNING: /* when the global variable State is in RUNNING the Task2 will elaborate the trigger to produce the PWM out */

        /* Change menu state if encoder button is clicked */
        if (g_rotaryEncoder.isEncoderButtonClicked()) 
        {
          /* If screensaver is active (timeout exceeded), just wake up - don't process menu action */
          if (millis() - g_lastEncoderInteraction > SCREENSAVER_TIMEOUT_MS) {
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
          *g_encoderSelectedValuePtr = g_encoderSecondarySelector;  /* Also update the value of the selected parameter */
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
      
      /* Apply motor control (skip if in calibration or init state) */
      if (!(g_currState == CALIBRATION || g_currState == INIT)) {
        if (g_escVar.trigger_norm == 0) {
          /* Apply brake when trigger is released */
          HalfBridge_SetPwmDrag(0, g_storedVar.carParam[g_carSel].brake);
          g_escVar.outputSpeed_pct = 0;
          throttleAntiSpin3(0);  /* Keep anti-spin timer updated */
        } else {
          /* Apply throttle curve and anti-spin */
          g_escVar.outputSpeed_pct = throttleCurve2(g_escVar.trigger_norm);
          g_escVar.outputSpeed_pct = throttleAntiSpin3(g_escVar.outputSpeed_pct);
          HalfBridge_SetPwmDrag(g_escVar.outputSpeed_pct, 0);
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
    g_storedVar.carParam[i].dragBrake = DRAG_BRAKE_DEFAULT;
    g_storedVar.carParam[i].maxSpeed = MAX_SPEED_DEFAULT;
    g_storedVar.carParam[i].throttleCurveVertex = { THROTTLE_CURVE_INPUT_THROTTLE_DEFAULT, THROTTLE_CURVE_SPEED_DIFF_DEFAULT };
    g_storedVar.carParam[i].antiSpin = ANTISPIN_DEFAULT;
    g_storedVar.carParam[i].freqPWM = PWM_FREQ_DEFAULT;
    g_storedVar.carParam[i].carNumber = i;
    sprintf(g_storedVar.carParam[i].carName, "CAR%1d", i);
  }
  g_storedVar.selectedCarNumber = 0;
  g_storedVar.minTrigger_raw = 0;
  g_storedVar.maxTrigger_raw = ACD_RESOLUTION_STEPS;
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
  /* Init menu items */

  sprintf(g_mainMenu.item[i].name, "BRAKE");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].brake;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  g_mainMenu.item[i].unit = '%';
  g_mainMenu.item[i].maxValue = BRAKE_MAX_VALUE;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "SENSI");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].minSpeed;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  g_mainMenu.item[i].unit = '%';
  g_mainMenu.item[i].maxValue = min(MIN_SPEED_MAX_VALUE, (int)g_storedVar.carParam[g_carSel].maxSpeed);
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "ANTIS");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].antiSpin;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  g_mainMenu.item[i].unit = 'm';
  g_mainMenu.item[i].maxValue = ANTISPIN_MAX_VALUE;
  g_mainMenu.item[i].minValue = 0;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "CURVE");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].throttleCurveVertex.curveSpeedDiff;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  g_mainMenu.item[i].unit = '%';
  g_mainMenu.item[i].maxValue = THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE;
  g_mainMenu.item[i].minValue = THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE;
  g_mainMenu.item[i].callback = &showCurveSelection;

  sprintf(g_mainMenu.item[++i].name, "PWM_F");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].freqPWM;
  g_mainMenu.item[i].type = VALUE_TYPE_DECIMAL;
  g_mainMenu.item[i].unit = 'k';
  g_mainMenu.item[i].maxValue = FREQ_MAX_VALUE / 100;
  g_mainMenu.item[i].minValue = FREQ_MIN_VALUE / 100;
  g_mainMenu.item[i].decimalPoint = 1;
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "LIMIT");
  g_mainMenu.item[i].value = (void *)&g_storedVar.carParam[g_carSel].maxSpeed;
  g_mainMenu.item[i].type = VALUE_TYPE_INTEGER;
  g_mainMenu.item[i].unit = '%';
  g_mainMenu.item[i].maxValue = MAX_SPEED_DEFAULT;
  g_mainMenu.item[i].minValue = max(5, (int)g_storedVar.carParam[g_carSel].minSpeed + 5);
  g_mainMenu.item[i].callback = ITEM_NO_CALLBACK;

  sprintf(g_mainMenu.item[++i].name, "*CAR*");
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
 * Show the Welcome screen
 */
void showScreenWelcome() 
{
  obdWriteString(&g_obd, 0, 16, 12, (char *)"ESPEED32", FONT_12x16, OBD_BLACK, 1);
  obdWriteString(&g_obd, 0, 16, 28, (char *)"BRM", FONT_12x16, OBD_BLACK, 1);
  sprintf(msgStr, "V%d.%02d", SW_MAJOR_VERSION, SW_MINOR_VERSION);  
  obdWriteString(&g_obd, 0, 16, 44, msgStr, FONT_12x16, OBD_WHITE, 1);
}


/**
 * Show the Pre-Calibration screen, when the button is pressed during startup, but not yet released
 */
void showScreenPreCalibration() 
{
  sprintf(msgStr, "ESPEED32 v%d.%02d", SW_MAJOR_VERSION, SW_MINOR_VERSION);  //print SW version
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
  sprintf(msgStr, "ESPEED32 v%d.%02d", SW_MAJOR_VERSION, SW_MINOR_VERSION);  //print SW version
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
  sprintf(msgStr, "CALIBRATION");
  obdWriteString(&g_obd, 0, (OLED_WIDTH - 66) / 2, 0, msgStr, FONT_6x8, OBD_WHITE, 1);

  sprintf(msgStr, "press/releas throttle");
  obdWriteString(&g_obd, 0, 0, 8, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Raw throttle %4d  ", adcRaw);
  obdWriteString(&g_obd, 0, 0, 24, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Min throttle %4d   ", g_storedVar.minTrigger_raw);
  obdWriteString(&g_obd, 0, 0, 32, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, "Max throttle %4d   ", g_storedVar.maxTrigger_raw);
  obdWriteString(&g_obd, 0, 0, 40, msgStr, FONT_6x8, OBD_BLACK, 1);

  sprintf(msgStr, " push when done ");
  obdWriteString(&g_obd, 0, 0, 56, msgStr, FONT_6x8, OBD_BLACK, 1);
}

/**
 * @brief Display bottom status line with throttle, car name, D indicator and voltage
 * @details Common function used by both main menu and screensaver
 */
void displayStatusLine() {
  /* Throttle % - left (position 0) */
  sprintf(msgStr, "%3d%c", g_escVar.outputSpeed_pct, '%');  
  obdWriteString(&g_obd, 0, 0, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_8x8, (g_escVar.outputSpeed_pct == 100) ? OBD_WHITE : OBD_BLACK, 1);
  
  /* Car ID - second from left (position 4*8 = 32 pixels) */
  sprintf(msgStr, "%s", g_storedVar.carParam[g_carSel].carName);
  obdWriteString(&g_obd, 0, 4 * WIDTH8x8, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);
  
  /* Show 'D' if Drag Brake is active - center (position 7.5*8 = 60 pixels) */
  if (g_storedVar.carParam[g_carSel].dragBrake > 100 - (uint16_t)g_storedVar.carParam[g_carSel].minSpeed) {
    g_escVar.dualCurve = true;
    sprintf(msgStr, "D");
    obdWriteString(&g_obd, 0, 60, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);
  } else {
    /* Clear the 'D' indicator if no dual curve */
    g_escVar.dualCurve = false;
    sprintf(msgStr, " ");
    obdWriteString(&g_obd, 0, 60, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);
  }
  
  /* Motor current - second from right (position 72 pixels) */
  sprintf(msgStr, "%d.%01dA", g_escVar.motorCurrent_mA / 1000, (g_escVar.motorCurrent_mA % 1000) / 100);
  obdWriteString(&g_obd, 0, 72, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);
  
  /* Input voltage - right aligned */
  sprintf(msgStr, "%d.%01dV", g_escVar.Vin_mV / 1000, (g_escVar.Vin_mV % 1000) / 100);
  obdWriteString(&g_obd, 0, OLED_WIDTH - 30, 3 * HEIGHT12x16 + HEIGHT8x8, msgStr, FONT_6x8, OBD_BLACK, 1);
}

/**
 * @brief Show screensaver with branding
 * @details Displays "RCW Racing" logo with LIMITER warning and status line
 */
void showScreensaver() {
  /* Clear screen */
  obdFill(&g_obd, OBD_WHITE, 1);
  
  /* Display "RCW" in extra large font centered */
  /* obdWriteString(&g_obd, 0, 10, 8, (char *)"Vandaas", FONT_16x32, OBD_BLACK, 1); */
  obdWriteString(&g_obd, 0, (OLED_WIDTH - 48) / 2, 8, (char *)"RCW", FONT_16x32, OBD_BLACK, 1);
  
  /* Display "Racing" in smaller font centered below */
  obdWriteString(&g_obd, 0, (OLED_WIDTH - 36) / 2, 34, (char *)"Racing", FONT_6x8, OBD_BLACK, 1);
  
  /* Display LIMITER warning if active at same position as in menu */
  if (g_storedVar.carParam[g_carSel].maxSpeed < MAX_SPEED_DEFAULT) 
  {
    obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)" - LIMITER - ", FONT_8x8, OBD_WHITE, 1);
  }
  
  /* Display bottom status line */
  displayStatusLine();
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
     The difference between upper and lower bound is fixed to be g_mainMenu.lines
     It's important that the encoder boundaries matches the menu items (e.g., 8 items, encoder boundaries must be [1,8]) */
  static uint16_t frameUpper = 1;
  static uint16_t frameLower = g_mainMenu.lines;

  /* In encoder move out of frame, adjust frame */
  if (g_encoderMainSelector > frameLower) 
  {
    frameLower = g_encoderMainSelector;
    frameUpper = frameLower - g_mainMenu.lines + 1;
    obdFill(&g_obd, OBD_WHITE, 1);
    screensaverActive = false;
  } 
  else if (g_encoderMainSelector < frameUpper) 
  {
    frameUpper = g_encoderMainSelector;
    frameLower = frameUpper + g_mainMenu.lines - 1;
    obdFill(&g_obd, OBD_WHITE, 1);
    screensaverActive = false;
  }

  /* Check if screensaver should be shown or menu */
  if (millis() - g_lastEncoderInteraction > SCREENSAVER_TIMEOUT_MS) 
  {
    /* Show screensaver */
    if (!screensaverActive) {
      screensaverActive = true;
      showScreensaver();
    }
  }
  /* Print main menu only if there was an encoder interaction recently */
  else if (millis() - g_lastEncoderInteraction < 100) 
  {
    if (screensaverActive) {
      obdFill(&g_obd, OBD_WHITE, 1);
      screensaverActive = false;
    }
    
    for (uint8_t i = 0; i < g_mainMenu.lines; i++)
    {
      /* Print item name */
      /* Item color: WHITE if item is selected, black otherwise */
      obdWriteString(&g_obd, 0, 0, i * HEIGHT12x16, g_mainMenu.item[frameUpper - 1 + i].name, FONT_12x16, (g_encoderMainSelector - frameUpper == i) ? OBD_WHITE : OBD_BLACK, 1);

      /* Only print value if value != ITEM_NO_VALUE */
      /* Value color: WHITE if corresponding item is selected AND menu state is VALUE_SELECTION, black otherwise */
      if (g_mainMenu.item[frameUpper - 1 + i].value != ITEM_NO_VALUE) 
      {
        /* if the value is a number, cast to *(unit16_t *), then print number and unit */
        if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_INTEGER) 
        {
          /* value is a generic pointer to void, so first cast to uint16_t pointer, then take the pointed value */
          sprintf(msgStr, "%4d%c", *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value), g_mainMenu.item[frameUpper - 1 + i].unit);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, i * HEIGHT12x16, msgStr, FONT_12x16, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
        /* If the value is a decimal, cast to *(unit16_t *), divide by 10^decimalPoint then print number and unit */
        else if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_DECIMAL) 
        {
          /* value is a generic pointer to void, so first cast to uint16_t pointer, then take the pointed value */
          tmp = *(uint16_t *)(g_mainMenu.item[frameUpper - 1 + i].value);
          sprintf(msgStr, " %d.%01d%c", tmp / 10, (tmp % 10), g_mainMenu.item[frameUpper - 1 + i].unit);
          obdWriteString(&g_obd, 0, OLED_WIDTH - 60, i * HEIGHT12x16, msgStr, FONT_12x16, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
        /* If the value is a string, cast to (char *) then print the string */
        else if (g_mainMenu.item[frameUpper - 1 + i].type == VALUE_TYPE_STRING) 
        {
          /* value is a generic pointer to void, so cast to string pointer */
          sprintf(msgStr, "%s", (char *)(g_mainMenu.item[frameUpper - 1 + i].value));
          obdWriteString(&g_obd, 0, OLED_WIDTH - (4 * WIDTH12x16), i * HEIGHT12x16, msgStr, FONT_12x16, (((g_encoderMainSelector - frameUpper == i) && (currMenuState == VALUE_SELECTION)) ? OBD_WHITE : OBD_BLACK), 1);
        }
      }
    }

    initMenuItems();  /* update menu items with the storedVar that could have been changed in previous for cycle */
    /* Print LIMITER warning if LIMIT is any value other than 100% */
    if (g_storedVar.carParam[g_carSel].maxSpeed < MAX_SPEED_DEFAULT) 
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)" - LIMITER - ", FONT_8x8, OBD_WHITE, 1);
    } 
    else 
    {
      obdWriteString(&g_obd, 0, WIDTH8x8, 3 * HEIGHT12x16, (char *)"             ", FONT_8x8, OBD_BLACK, 1);
    }
  }

  /* Display bottom status line */
  displayStatusLine();
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
  /* ignore multiple press in that are less than 200ms apart */
  if (millis() - lastTimePressed < 200)
  {
    return currMenuState;
  }
  
  lastTimePressed = millis();

  if (currMenuState == ITEM_SELECTION) /* If the current state is ITEM_SELECTION */
  {
    /* If an item has no callback, go in menu state VALUE_SELECTION */
    if (g_mainMenu.item[g_encoderMainSelector - 1].callback == ITEM_NO_CALLBACK) 
    {
      g_rotaryEncoder.setAcceleration(SEL_ACCELERATION); /* Set higher encoder acceleration so it doesn't require too many turns to make a big value change */
      
      
      g_encoderSelectedValuePtr = (uint16_t *)g_mainMenu.item[g_encoderMainSelector - 1].value;   /* Update the g_encoderSelectedValuePtr to point to the value of the selected item.
                                                                                                     value is a generic pointer to void, so cast to uint16_t pointer */
      selectedParamMaxValue = g_mainMenu.item[g_encoderMainSelector - 1].maxValue;                /* Set Max and Min boundaries according to the selected items max and min value */
      selectedParamMinValue = g_mainMenu.item[g_encoderMainSelector - 1].minValue;
      g_rotaryEncoder.setBoundaries(selectedParamMinValue, selectedParamMaxValue, false);
      g_rotaryEncoder.reset(*g_encoderSelectedValuePtr);  /* Reset the encoder to the current value of the selected item */
      g_escVar.encoderPos = *g_encoderSelectedValuePtr;   /* Set the encoderPos global variable to the current value of the selected item */
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
    g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);  /* Set the encoder boundaries to the menu boundaries */
    g_rotaryEncoder.reset(g_encoderMainSelector);               /* Reset the encoder value to g_encoderMainSelector, so that it doesn't change the selected item */
    g_escVar.encoderPos = g_encoderMainSelector;
    
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

  /* Set encoder to car selection parameter */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(0, CAR_MAX_COUNT - 1, false);
  g_rotaryEncoder.reset(g_storedVar.selectedCarNumber);

  /* Exit car selection when encoder is clicked */
  while (!g_rotaryEncoder.isEncoderButtonClicked()) 
  {
    /* Get encoder value if changed */
    g_storedVar.selectedCarNumber = g_rotaryEncoder.encoderChanged() ? g_rotaryEncoder.readEncoder() : g_storedVar.selectedCarNumber;
    
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

    /* Print "-SELECT THE CAR-" on the bottom of the screen */
    obdWriteString(&g_obd, 0, 16, OLED_HEIGHT - HEIGHT8x8, (char *)"-SELECT THE CAR-", FONT_6x8, OBD_WHITE, 1);
  }

  /* Reset encoder and clear is done in caller routine */
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
  g_rotaryEncoder.setBoundaries(0, 1, false); /* Boundaries are [0, 1] because there are only two options */
  g_rotaryEncoder.reset(selectedOption);

  /* Print the "SELECT AN OPTION" */
  obdWriteString(&g_obd, 0, 16, OLED_HEIGHT - HEIGHT8x8, (char *)"-PICK AN OPTION-", FONT_6x8, OBD_WHITE, 1);

  /* Exit car selection when encoder is clicked */
  while (!g_rotaryEncoder.isEncoderButtonClicked())
  {
    /* Get encoder value if changed */
    selectedOption = g_rotaryEncoder.encoderChanged() ? g_rotaryEncoder.readEncoder() : selectedOption;
    /* Print the two options */
    obdWriteString(&g_obd, 0, 0, 0 * HEIGHT12x16, (char *)"SELECT", FONT_12x16, (selectedOption == CAR_OPTION_SELECT) ? OBD_WHITE : OBD_BLACK, 1);
    obdWriteString(&g_obd, 0, 0, 1 * HEIGHT12x16, (char *)"RENAME", FONT_12x16, (selectedOption == CAR_OPTION_RENAME) ? OBD_WHITE : OBD_BLACK, 1);
  }

  /* If RENAME option was selected, go to renameCar routine */
  if (selectedOption == CAR_OPTION_RENAME) 
  {
    showRenameCar();
    saveEEPROM(g_storedVar);
  }
  /* If SELECT option was selected, go to showCarSelection routine */
  else if (selectedOption == CAR_OPTION_SELECT) 
  {
    showCarSelection();
    saveEEPROM(g_storedVar);
  }

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
  obdWriteString(&g_obd, 0, 16, 0, (char *)"-RENAME THE CAR-", FONT_6x8, OBD_WHITE, 1);
  obdWriteString(&g_obd, 0, 1, OLED_HEIGHT - HEIGHT8x8, (char *)"-CLICK OK TO CONFIRM-", FONT_6x8, OBD_WHITE, 1);

  /* Draw the right arrow */
  for (uint8_t j = 0; j < 8; j++) 
  {
    obdDrawLine(&g_obd, 80 + j, 16 + j, 80 + j, 30 - j, OBD_BLACK, 1);
  }
  obdDrawLine(&g_obd, 72, 22, 80, 22, OBD_BLACK, 1);
  obdDrawLine(&g_obd, 72, 23, 80, 23, OBD_BLACK, 1);
  obdDrawLine(&g_obd, 72, 24, 80, 24, OBD_BLACK, 1);

  /* Exit car renaming when encoder is clicked AND CONFIRM is selected */
  while (1) 
  {
    /* Get encoder value if changed */
    /* Change selectedOption if in RENAME_CAR_SELECT_OPTION_MODE */
    if (mode == RENAME_CAR_SELECT_OPTION_MODE) 
    {
      selectedOption = g_rotaryEncoder.encoderChanged() ? g_rotaryEncoder.readEncoder() : selectedOption;
    }
    /* Change selectedChar if in RENAME_CAR_SELECT_CHAR_MODE */
    if (mode == RENAME_CAR_SELECT_CHAR_MODE) 
    {
      selectedChar = g_rotaryEncoder.encoderChanged() ? g_rotaryEncoder.readEncoder() : selectedChar;
      tmpName[selectedOption] = (char)selectedChar; /* Change the value of the selected char in the temp name */

      /* Draw the upward and downward arrows on the selected char to indicate that it can be changed */
      for (uint8_t j = 0; j < 6; j++) 
      {
        obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 14 - j, 11 - j + (selectedOption * 12), 14 - j, OBD_BLACK, 1);
        obdDrawLine(&g_obd, 1 + j + (selectedOption * 12), 33 + j, 11 - j + (selectedOption * 12), 33 + j, OBD_BLACK, 1);
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
  while (!g_rotaryEncoder.isEncoderButtonClicked())
  {
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
  }

  /* Reset encoder */
  g_rotaryEncoder.setAcceleration(MENU_ACCELERATION);
  g_rotaryEncoder.setBoundaries(1, MENU_ITEMS_COUNT, false);
  g_rotaryEncoder.reset(g_encoderMainSelector);
  g_escVar.encoderPos = g_encoderMainSelector;
  saveEEPROM(g_storedVar);          /* Save modified values to EEPROM */
  obdFill(&g_obd, OBD_WHITE, 1);    /* Clear screen */

  return;
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