#ifndef _GROWATT_1_20_H_
#define _GROWATT_1_20_H_

#include "Arduino.h"

#include "GrowattTypes.h"

// Growatt modbus protocol version 1.20
typedef enum
{
    P120_I_STATUS = 0,
    P120_INPUT_POWER,
    P120_PV1_VOLTAGE,
    P120_PV1_INPUT_POWER,
    P120_PV1_INPUT_CURRENT,
    P120_PV2_VOLTAGE,
    P120_PV2_INPUT_POWER,
    P120_PV2_INPUT_CURRENT,
    P120_OUTPUT_POWER,
    P120_GRID_FREQUENCY,
    P120_GRID_L1_VOLTAGE,
    P120_GRID_L1_OUTPUT_CURRENT,
    P120_GRID_L1_OUTPUT_POWER,
    P120_GRID_L2_VOLTAGE,
    P120_GRID_L2_OUTPUT_CURRENT,
    P120_GRID_L2_OUTPUT_POWER,
    P120_GRID_L3_VOLTAGE,
    P120_GRID_L3_OUTPUT_CURRENT,
    P120_GRID_L3_OUTPUT_POWER,
    P120_ENERGY_TODAY,
    P120_ENERGY_TOTAL,
    P120_WORK_TIME_TOTAL,
    P120_PV1_ENERGY_TODAY,
    P120_PV1_ENERGY_TOTAL,
    P120_PV2_ENERGY_TODAY,
    P120_PV2_ENERGY_TOTAL,
    P120_PV_ENERGY_TOTAL,
    P120_INVERTER_TEMPERATURE,
    P120_INVERTER_IPM_TEMPERATURE,
} eP120InputRegisters_t;

typedef enum
{
    P120_OnOff,            // Register 0
    P120_CMD_MEMORY_STATE, // Register 2
    P120_Active_P_Rate     // Register 3
} eP120HoldingRegisters_t;

void init_growatt120(sProtocolDefinition_t &Protocol);

#endif // _GROWATT_1_20_H_