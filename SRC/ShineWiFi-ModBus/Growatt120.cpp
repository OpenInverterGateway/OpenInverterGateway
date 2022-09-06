#include "Arduino.h"

#include "Growatt120.h"

// Supported inverters:
// - Growatt MIC 1000TL-X  

void init_growatt120(sProtocolDefinition_t &Protocol) {
    // definition of input registers
    Protocol.InputRegisterCount = 12;
    Protocol.InputFragmentCount = 3;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 6};
    Protocol.InputReadFragments[1] = sGrowattReadFragment_t{30, 30};
    Protocol.InputReadFragments[2] = sGrowattReadFragment_t{3093, 1};

    // FRAGMENT 1: BEGIN
    // address, value, size, name, multiplier, unit, frontend, plot
    Protocol.InputRegisters[P120_I_STATUS] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "InverterStatus", 1, NONE, true, false}; // #1
    Protocol.InputRegisters[P120_DC_POWER] = sGrowattModbusReg_t{5, 0, SIZE_32BIT, "DcPower", 0.1, POWER_W, true, true}; // #2
    Protocol.InputRegisters[P120_DC_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "DcVoltage", 0.1, VOLTAGE, true, false}; // #3
    Protocol.InputRegisters[P120_DC_INPUT_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "DcInputCurrent", 0.1, CURRENT, true, false}; // #4
    // FRAGMENT 1: END

     // FRAGMENT 2: BEGIN
    // address, value, size, name, multiplier, unit, frontend, plot
    Protocol.InputRegisters[P120_AC_FREQUENCY] = sGrowattModbusReg_t{37, 0, SIZE_16BIT, "AcFrequency", 0.01, FREQUENCY, true, false}; // #5
    Protocol.InputRegisters[P120_AC_VOLTAGE] = sGrowattModbusReg_t{38, 0, SIZE_16BIT, "AcVoltage", 0.1, VOLTAGE, true, false}; // #6
    Protocol.InputRegisters[P120_AC_OUTPUT_CURRENT] = sGrowattModbusReg_t{39, 0, SIZE_16BIT, "AcOutputCurrent", 0.1, CURRENT, true, false}; // #7
    Protocol.InputRegisters[P120_AC_POWER] = sGrowattModbusReg_t{40, 0, SIZE_32BIT, "AcPower", 0.1, POWER_W, true, true}; // #8
    Protocol.InputRegisters[P120_ENERGY_TODAY] = sGrowattModbusReg_t{53, 0, SIZE_32BIT, "EnergyToday", 0.1, POWER_KWH, true, false}; // #9
    Protocol.InputRegisters[P120_ENERGY_TOTAL] = sGrowattModbusReg_t{55, 0, SIZE_32BIT, "EnergyTotal", 0.1, POWER_KWH, true, false}; // #10
    Protocol.InputRegisters[P120_OPERATING_TIME] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "OperatingTime", 0.5, SECONDS, true, false}; // #11
    // FRAGMENT 2: END

    // FRAGMENT 3: BEGIN
    // address, value, size, name, multiplier, unit, frontend, plot
    Protocol.InputRegisters[P120_TEMPERATURE] = sGrowattModbusReg_t{3093, 0, SIZE_16BIT, "Temperature", 0.1, TEMPERATURE, true, false}; // #12
    // FRAGMENT 3: END

    // definition of holding registers
    Protocol.HoldingRegisterCount = 3;
    Protocol.HoldingFragmentCount = 1;
    Protocol.HoldingReadFragments[0] = sGrowattReadFragment_t{0, 4};

    // FRAGMENT 1: BEGIN
    Protocol.HoldingRegisters[P120_OnOff] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "OnOff", 1, NONE, true, false}; // #1
    Protocol.HoldingRegisters[P120_CMD_MEMORY_STATE] = sGrowattModbusReg_t{2, 0, SIZE_16BIT, "CmcMemoryState", 1, NONE, true, false}; // #2
    Protocol.HoldingRegisters[P120_Active_P_Rate] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "ActivePowerRate", 1, PRECENTAGE, true, false}; // #3
    // FRAGMENT 1: END
}
