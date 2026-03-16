#include "ext_pot.h"
#include <Arduino.h>
#include "slot_ESC.h"
#include "HAL.h"

extern StoredVar_type g_storedVar;
extern ESC_type g_escVar;
extern uint16_t g_carSel;

uint16_t g_extPotEnabled = EXT_POT_ENABLED_DEFAULT;
uint16_t g_extPotTarget = EXT_POT_TARGET_DEFAULT;

static uint32_t g_extPotFilteredRaw = 0;
static bool g_extPotFilterInit = false;

static uint16_t readExtPotFilteredRaw() {
  uint16_t raw = HAL_ReadExternalPotRaw();
  if (!g_extPotFilterInit) {
    g_extPotFilteredRaw = raw;
    g_extPotFilterInit = true;
  } else {
    g_extPotFilteredRaw = ((g_extPotFilteredRaw * 7UL) + raw) / 8UL;
  }
  return (uint16_t)g_extPotFilteredRaw;
}

static uint16_t mapExtPotToBrakePct(uint16_t raw) {
  return (uint16_t)map(raw, 0, ACD_RESOLUTION_STEPS, 0, BRAKE_MAX_VALUE);
}

static uint16_t mapExtPotToSensiRaw(uint16_t raw) {
  uint16_t maxSensiRaw = min((uint16_t)MIN_SPEED_MAX_VALUE, (uint16_t)(g_storedVar.carParam[g_carSel].maxSpeed * SENSI_SCALE));
  return (uint16_t)map(raw, 0, ACD_RESOLUTION_STEPS, 0, maxSensiRaw);
}

bool isExtPotEnabled() {
  return g_extPotEnabled != 0;
}

bool isExtPotBrakeTarget() {
  return isExtPotEnabled() && g_extPotTarget == EXT_POT_TARGET_BRAKE;
}

bool isExtPotSensiTarget() {
  return isExtPotEnabled() && g_extPotTarget == EXT_POT_TARGET_SENSI;
}

void resetExtPotFilter() {
  g_extPotFilterInit = false;
  g_extPotFilteredRaw = 0;
}

void updateExtPotRuntimeValues() {
  g_escVar.effectiveBrake_pct = g_storedVar.carParam[g_carSel].brake;
  g_escVar.effectiveSensi_raw = g_storedVar.carParam[g_carSel].minSpeed;

  if (!isExtPotEnabled()) return;

  uint16_t filteredRaw = readExtPotFilteredRaw();
  if (g_extPotTarget == EXT_POT_TARGET_BRAKE) {
    g_escVar.effectiveBrake_pct = constrain(mapExtPotToBrakePct(filteredRaw), 0, BRAKE_MAX_VALUE);
  } else if (g_extPotTarget == EXT_POT_TARGET_SENSI) {
    g_escVar.effectiveSensi_raw = constrain(mapExtPotToSensiRaw(filteredRaw), 0,
                                            min((uint16_t)MIN_SPEED_MAX_VALUE, (uint16_t)(g_storedVar.carParam[g_carSel].maxSpeed * SENSI_SCALE)));
  }
}

uint16_t getEffectiveBrakePct() {
  return isExtPotBrakeTarget() ? g_escVar.effectiveBrake_pct : g_storedVar.carParam[g_carSel].brake;
}

uint16_t getEffectiveSensiRaw() {
  return isExtPotSensiTarget() ? g_escVar.effectiveSensi_raw : g_storedVar.carParam[g_carSel].minSpeed;
}
