#ifndef _GROWATT_3_05_H_
#define _GROWATT_3_05_H_

#include "Arduino.h"

#include "GrowattTypes.h"

// Growatt modbus protocol version 1.24 from 2020-08-04
typedef enum
{
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
} eP305InputRegisters_t;

void init_growatt305(sProtocolDefinition_t& Protocol);

#endif // _GROWATT_3_05_H_
