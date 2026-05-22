#pragma once

#include "Arduino.h"
#include "Growatt.h"
#include "GrowattTypes.h"

// Growatt MODBUS protocol V3.05 (2013-04-25)
// Input register indices into Protocol.InputRegisters[]. Must remain sorted
// by Modbus address (see Growatt::ReadInputRegisters j-walk).
// Only block 1 (regs 0-44) is mapped here. Block 2 (47-89) is read raw from
// the .ino for the cloud sender because this inverter's FC04 response shifts
// block-2 data by +2 registers, which the standard fragment math can't model.
typedef enum {
  P305_I_STATUS = 0,       // R0
  P305_DC_POWER,           // R1-2  (Ppv total, U32)
  P305_DC_VOLTAGE,         // R3
  P305_DC_INPUT_CURRENT,   // R4
  P305_PV1_POWER,          // R5-6  (U32)
  P305_PV2_VOLTAGE,        // R7
  P305_PV2_CURRENT,        // R8
  P305_PV2_POWER,          // R9-10 (U32)
  P305_PAC_TOTAL,          // R11-12 (total AC output power, U32)
  P305_AC_FREQUENCY,       // R13
  P305_AC_VOLTAGE,         // R14  (L1)
  P305_AC_OUTPUT_CURRENT,  // R15  (L1)
  P305_AC_POWER,           // R16-17 (Pac L1, U32 - keeps legacy name)
  P305_AC_VOLTAGE_L2,      // R18
  P305_AC_CURRENT_L2,      // R19
  P305_AC_POWER_L2,        // R20-21 (U32)
  P305_AC_VOLTAGE_L3,      // R22
  P305_AC_CURRENT_L3,      // R23
  P305_AC_POWER_L3,        // R24-25 (U32)
  P305_ENERGY_TODAY,       // R26-27 (U32)
  P305_ENERGY_TOTAL,       // R28-29 (U32)
  P305_OPERATING_TIME,     // R30-31 (U32)
  P305_TEMPERATURE,        // R32
  P305_ISO_FAULT,          // R33
  P305_GFCI_FAULT,         // R34
  P305_DCI_FAULT,          // R35
  P305_VPV_FAULT,          // R36
  P305_VAC_FAULT,          // R37
  P305_FAC_FAULT,          // R38
  P305_TMP_FAULT,          // R39
  P305_FAULT_CODE,         // R40
  P305_IPM_TEMPERATURE,    // R41
  P305_PBUS_VOLTAGE,       // R42
  P305_NBUS_VOLTAGE,       // R43
} eP305InputRegisters_t;

void init_growatt305(sProtocolDefinition_t& Protocol, Growatt& inverter);
