#pragma once

#include "Arduino.h"
#include "Growatt.h"
#include "GrowattTypes.h"

// Growatt MODBUS protocol V3.05 (2013-04-25)
typedef enum {
  P305_I_STATUS = 0,
  P305_DC_POWER,
  P305_DC_VOLTAGE,
  P305_DC_INPUT_CURRENT,
  P305_AC_FREQUENCY,
  P305_AC_VOLTAGE,
  P305_AC_OUTPUT_CURRENT,
  P305_AC_POWER,
  P305_ENERGY_TODAY,
  P305_ENERGY_TOTAL,
  P305_OPERATING_TIME,
  P305_TEMPERATURE,
  P305_PAC_TOTAL,    // reg 11-12, total AC output power (cloud "pac")
  P305_PV_POWER_IN,  // reg 1-2, total PV input power
  P305_PV1_WATT,     // reg 5-6, PV1 power
  P305_VAC_L2,       // reg 18
  P305_IAC_L2,       // reg 19
  P305_PAC_L2,       // reg 20-21, L2 power
  P305_VAC_L3,       // reg 22
  P305_IAC_L3,       // reg 23
  P305_PAC_L3,       // reg 24-25, L3 power
} eP305InputRegisters_t;

void init_growatt305(sProtocolDefinition_t& Protocol, Growatt& inverter);
