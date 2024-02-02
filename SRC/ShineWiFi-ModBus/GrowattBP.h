#pragma once

#include "Arduino.h"
#include "Growatt.h"
#include "GrowattTypes.h"

// Growatt modbus protocol for BP series
typedef enum {
  // general
  BP_I_STATUS = 0,
  BP_INPUT_POWER,
  BP_OUTPUT_POWER,

  // input 1
  BP_PV1_VOLTAGE,
  BP_PV1_CURRENT,
  BP_PV1_POWER,

  // input 2
  BP_PV2_VOLTAGE,
  BP_PV2_CURRENT,
  BP_PV2_POWER,

  // grid and phase
  BP_GRID_FREQUENCY,
  BP_AC1_VOLTAGE,
  BP_AC1_CURRENT,
  BP_AC1_POWER,
  BP_AC2_VOLTAGE,
  BP_AC2_CURRENT,
  BP_AC2_POWER,
  BP_AC3_VOLTAGE,
  BP_AC3_CURRENT,
  BP_AC3_POWER,

  // statistics
  BP_EAC_TODAY,
  BP_EAC_TOTAL,
  BP_TIME_TOTAL,
  BP_EPV1_TODAY,
  BP_EPV1_TOTAL,
  BP_EPV2_TODAY,
  BP_EPV2_TOTAL,
  BP_EPV_TOTAL,

  // temperature
  BP_TEMP1,
  BP_TEMP2,

  // battery
  BP_BAT_PERCENTAGE,
  BP_BAT_CHARGE_POWER,
  BP_BAT_DISCHARGE_POWER,
} eP305InputRegisters_t;

void init_growattBP(sProtocolDefinition_t& Protocol, Growatt& inverter);