#ifndef _GROWATT_1_20_H_
#define _GROWATT_1_20_H_

#include "Arduino.h"

#include "GrowattTypes.h"

// Growatt modbus protocol version 1.20
typedef enum {
    P120_I_STATUS = 0,
    P120_DC_POWER,
    P120_DC_VOLTAGE,
    P120_DC_INPUT_CURRENT,
    P120_AC_FREQUENCY,
    P120_AC_VOLTAGE,
    P120_AC_OUTPUT_CURRENT,
    P120_AC_POWER,
    P120_ENERGY_TODAY,
    P120_ENERGY_TOTAL,
    P120_OPERATING_TIME,
    P120_TEMPERATURE,
} eP120InputRegisters_t;


typedef enum {
    P120_OnOff, // Register 0
    P120_CMD_MEMORY_STATE, // Register 2
    P120_Active_P_Rate // Register 3
} eP120HoldingRegisters_t;

void init_growatt120(sProtocolDefinition_t &Protocol);

#endif // _GROWATT_1_20_H_