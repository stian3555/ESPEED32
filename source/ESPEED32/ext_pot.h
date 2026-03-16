#ifndef EXT_POT_H_
#define EXT_POT_H_

#include <stdint.h>

extern uint16_t g_extPotEnabled;
extern uint16_t g_extPotTarget;

bool isExtPotEnabled();
bool isExtPotBrakeTarget();
bool isExtPotSensiTarget();
void resetExtPotFilter();
void updateExtPotRuntimeValues();
uint16_t getEffectiveBrakePct();
uint16_t getEffectiveSensiRaw();

#endif  /* EXT_POT_H_ */
