#ifndef SLOT_ESC_H_
#define SLOT_ESC_H_

/*********************************************************************************************************************/
/*                                                   Includes                                                        */
/*********************************************************************************************************************/
#include <OneBitDisplay.h>
#include <AiEsp32RotaryEncoder.h>
#include <AiEsp32RotaryEncoderNumberSelector.h>
#include "half_bridge.h"
#include "HAL.h"
#include <Preferences.h>

/*********************************************************************************************************************/
/*                                              Configuration Constants                                             */
/*********************************************************************************************************************/

/* Menu Configuration */
#define MENU_ITEMS_COUNT    7     /* Number of items in main menu */
#define MENU_ACCELERATION   0     /* Encoder acceleration in menu navigation */
#define SEL_ACCELERATION    100   /* Encoder acceleration when adjusting values */
#define ITEM_NO_CALLBACK    0     /* Indicates menu item has no callback function */
#define ITEM_NO_VALUE       0     /* Indicates menu item has no displayable value */
#define MAX_ITEMS           10    /* Maximum items in any menu */

/* Default Parameter Values */
#define MIN_SPEED_DEFAULT         20    /* [%] Minimum motor speed (sensitivity) */
#define BRAKE_DEFAULT             95    /* [%] Brake strength */
#define DRAG_BRAKE_DEFAULT        100   /* [%] Drag brake strength */
#define ANTISPIN_DEFAULT          30    /* [ms] Anti-spin ramp time */
#define MAX_SPEED_DEFAULT         100   /* [%] Maximum motor speed */
#define THROTTLE_CURVE_INPUT_THROTTLE_DEFAULT   (THROTTLE_NORMALIZED / 2)  /* Throttle curve vertex X */
#define THROTTLE_CURVE_SPEED_DIFF_DEFAULT       50                          /* Throttle curve vertex Y */
#define PWM_FREQ_DEFAULT          30    /* [100*Hz] Motor PWM frequency (3.0 kHz) */

/* Parameter Limits */
#define MIN_SPEED_MAX_VALUE       90    /* [%] Maximum allowed minimum speed */
#define DRAG_MAX_VALUE            100   /* [%] Maximum drag brake */
#define FREQ_MAX_VALUE            5000  /* [Hz] Maximum PWM frequency */
#define BRAKE_MAX_VALUE           100   /* [%] Maximum brake strength */
#define THROTTLE_CURVE_SPEED_DIFF_MAX_VALUE  90   /* [%] Throttle curve max */
#define THROTTLE_CURVE_SPEED_DIFF_MIN_VALUE  10   /* [%] Throttle curve min */
#define ANTISPIN_MAX_VALUE        255   /* [ms] Maximum anti-spin time */
#define FREQ_MIN_VALUE            1000  /* [Hz] Minimum PWM frequency */
#define MAX_UINT16                32767 /* Maximum 16-bit unsigned value */

/* Display Font Sizes */
#define HEIGHT12x16     16
#define HEIGHT8x8       8
#define WIDTH8x8        8
#define WIDTH12x16      12

/* Timing Constants */
#define ESC_PERIOD_US   500     /* ESC control loop period [µs] */
#define SCREENSAVER_TIMEOUT_MS  5000  /* [ms] Time before screensaver activates */

/* Car Configuration */
#define CAR_MAX_COUNT       10  /* Maximum number of car profiles */
#define CAR_NAME_MAX_SIZE   5   /* Car name length (4 chars + null terminator) */

/* Menu Options */
#define CAR_OPTION_SELECT   0
#define CAR_OPTION_RENAME   1

/* Rename Car Mode States */
#define RENAME_CAR_SELECT_OPTION_MODE   0
#define RENAME_CAR_SELECT_CHAR_MODE     1
#define RENAME_CAR_MIN_ASCII    33      /* First printable ASCII character */
#define RENAME_CAR_MAX_ASCII    122     /* Last lowercase letter */
/*********************************************************************************************************************/
/*                                                 Data Structures                                                   */
/*********************************************************************************************************************/

/**
 * @brief Main state machine states
 */
typedef enum { 
  INIT,         /* Initialization state */
  CALIBRATION,  /* Trigger calibration mode */
  WELCOME,      /* Welcome screen display */
  RUNNING,      /* Normal operation */
  FAULT         /* Fault state (currently unused) */
} StateMachine_enum;

/**
 * @brief Menu navigation states
 */
typedef enum {
  ITEM_SELECTION,   /* Selecting which menu item to modify */
  VALUE_SELECTION   /* Adjusting the value of selected item */
} MenuState_enum;

/**
 * @brief Menu item value types
 * @note Decimal values: Value is stored as integer and divided by 10^decimalPoint for display
 *       Example: PWM freq 3.0 kHz is stored as 30 with decimalPoint=1
 */
typedef enum {
  VALUE_TYPE_INTEGER,   /* Whole number value */
  VALUE_TYPE_DECIMAL,   /* Decimal value (stored as integer) */
  VALUE_TYPE_STRING     /* String value */
} ItemValueType_enum;

/**
 * @brief Throttle curve vertex definition
 * @details The throttle curve consists of two line segments connected at a vertex point.
 *          X-axis = input throttle (0-100%), Y-axis = output speed (0-100%)
 */
typedef struct {
  uint16_t inputThrottle;      /* X coordinate of vertex (fixed at 50% by default) */
  uint16_t curveSpeedDiff;     /* Y coordinate as percentage of min-max speed difference */
} ThrottleCurveVertex_type;

/**
 * @brief Car profile parameters
 * @details Contains all ESC behavior settings for a specific car/track configuration
 */
typedef struct {
  uint16_t minSpeed;                        /* [%] Minimum motor speed (0-90%) */
  uint16_t brake;                           /* [%] Brake strength (0-100%) */
  uint16_t dragBrake;                       /* [%] Drag brake strength (0-100%) */
  uint16_t maxSpeed;                        /* [%] Maximum motor speed (5-100%) */
  ThrottleCurveVertex_type throttleCurveVertex;  /* Throttle response curve */
  uint16_t antiSpin;                        /* [ms] Anti-spin ramp time (0-255ms) */
  char carName[CAR_NAME_MAX_SIZE];          /* Car profile name (4 chars + null) */
  uint16_t carNumber;                       /* Profile index in array */
  uint16_t freqPWM;                         /* [100*Hz] Motor PWM frequency */
} CarParam_type;

/**
 * @brief Non-volatile stored variables
 * @details Parameters saved to flash memory (Preferences library)
 */
typedef struct {
  CarParam_type carParam[CAR_MAX_COUNT];    /* Array of car profiles */
  uint16_t selectedCarNumber;               /* Currently active car profile */
  int16_t minTrigger_raw;                   /* Calibrated minimum trigger value */
  int16_t maxTrigger_raw;                   /* Calibrated maximum trigger value */
} StoredVar_type;

/**
 * @brief ESC runtime variables
 * @details Real-time state variables updated during operation
 */
typedef struct {
  uint16_t outputSpeed_pct;   /* [%] Current motor duty cycle (0-100%) */
  int16_t trigger_raw;        /* Raw trigger sensor reading */
  uint16_t trigger_norm;      /* Normalized trigger value (0-THROTTLE_NORMALIZED) */
  uint16_t encoderPos;        /* Current rotary encoder position */
  uint16_t Vin_mV;            /* [mV] Input voltage */
  bool dualCurve;             /* True if deceleration uses different curve */
} ESC_type;

/**
 * @brief Function pointer type for menu callbacks
 */
typedef void (*FunctionPointer_type)(void);

/**
 * @brief Menu item definition
 * @details Describes a single item in the menu system
 */
typedef struct {
  char name[10];                  /* Item name displayed in menu */
  void *value;                    /* Pointer to value (cast to appropriate type) */
  ItemValueType_enum type;        /* Value type for display formatting */
  uint16_t maxValue;              /* Maximum allowed value */
  uint16_t minValue;              /* Minimum allowed value */
  char unit;                      /* Measurement unit character */
  uint8_t decimalPoint;           /* Decimal point position (1 or 2) */
  FunctionPointer_type callback;  /* Function called when item is selected */
} MenuItem_type;

/**
 * @brief Menu structure
 */
typedef struct {
  MenuItem_type item[MAX_ITEMS];  /* Array of menu items */
  uint16_t lines;                 /* Number of visible lines */
} Menu_type;

#endif  /* SLOT_ESC_H_ */