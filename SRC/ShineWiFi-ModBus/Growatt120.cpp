#include "Arduino.h"

#include "Growatt120.h"

// Supported inverters:
// - Growatt MIC 1000TL-X

// Should also work for the following series (with input register range 0~124):
// - TL-X（MIN Type）
// - TL3-X(MAX、MID、MAC Type)
// - Storage(MIX Type)
// - Storage(SPA Type)
// - Storage(SPH Type)：

void init_growatt120(sProtocolDefinition_t& Protocol)
{
    // definition of input registers
    Protocol.InputRegisterCount = 29;
    Protocol.InputFragmentCount = 2;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 49};
    Protocol.InputReadFragments[1] = sGrowattReadFragment_t{53, 42};

    // FRAGMENT 1: BEGIN
    // address, value, size, name, multiplier, unit, frontend, plot
    Protocol.InputRegisters[P120_I_STATUS] = sGrowattModbusReg_t{0, 0,
SIZE_16BIT, "InverterStatus", 1, NONE, true, false};    // #1
    Protocol.InputRegisters[P120_INPUT_POWER] = sGrowattModbusReg_t{1, 0, SIZE_32BIT, "InputPower", 0.1, POWER_W, true, true}; // #2

    Protocol.InputRegisters[P120_PV1_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "PV1Voltage", 0.1, VOLTAGE, false, false};            // #3
    Protocol.InputRegisters[P120_PV1_INPUT_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "PV1InputCurrent", 0.1, CURRENT, false, false}; // #4
    Protocol.InputRegisters[P120_PV1_INPUT_POWER] = sGrowattModbusReg_t{5, 0, SIZE_32BIT, "PV1InputPower", 0.1, POWER_W, false, false};     // #5

    Protocol.InputRegisters[P120_PV2_VOLTAGE] = sGrowattModbusReg_t{7, 0, SIZE_16BIT, "PV2Voltage", 0.1, VOLTAGE, false, false};            // #6
    Protocol.InputRegisters[P120_PV2_INPUT_CURRENT] = sGrowattModbusReg_t{8, 0, SIZE_16BIT, "PV2InputCurrent", 0.1, CURRENT, false, false}; // #7
    Protocol.InputRegisters[P120_PV2_INPUT_POWER] = sGrowattModbusReg_t{9, 0, SIZE_32BIT, "PV2InputPower", 0.1, POWER_W, false, false};     // #8

    Protocol.InputRegisters[P120_OUTPUT_POWER] = sGrowattModbusReg_t{35, 0, SIZE_32BIT, "OutputPower", 0.1, POWER_W, true, true};          // #9
    Protocol.InputRegisters[P120_GRID_FREQUENCY] = sGrowattModbusReg_t{37, 0, SIZE_16BIT, "GridFrequency", 0.01, FREQUENCY, false, false}; // #10

    Protocol.InputRegisters[P120_GRID_L1_VOLTAGE] = sGrowattModbusReg_t{38, 0, SIZE_16BIT, "GridL1Voltage", 0.1, VOLTAGE, false, false};              // #11
    Protocol.InputRegisters[P120_GRID_L1_OUTPUT_CURRENT] = sGrowattModbusReg_t{39, 0, SIZE_16BIT, "GridL1OutputCurrent", 0.1, CURRENT, false, false}; // #12
    Protocol.InputRegisters[P120_GRID_L1_OUTPUT_POWER] = sGrowattModbusReg_t{40, 0, SIZE_32BIT, "GridL1OutputPower", 0.1, VA, false, false};          // #13

    Protocol.InputRegisters[P120_GRID_L2_VOLTAGE] = sGrowattModbusReg_t{42, 0, SIZE_16BIT, "GridL2Voltage", 0.1, VOLTAGE, false, false};              // #14
    Protocol.InputRegisters[P120_GRID_L2_OUTPUT_CURRENT] = sGrowattModbusReg_t{43, 0, SIZE_16BIT, "GridL2OutputCurrent", 0.1, CURRENT, false, false}; // #15
    Protocol.InputRegisters[P120_GRID_L2_OUTPUT_POWER] = sGrowattModbusReg_t{44, 0, SIZE_32BIT, "GridL2OutputPower", 0.1, VA, false, false};          // #16

    Protocol.InputRegisters[P120_GRID_L3_VOLTAGE] = sGrowattModbusReg_t{46, 0, SIZE_16BIT, "GridL3Voltage", 0.1, VOLTAGE, false, false};              // #17
    Protocol.InputRegisters[P120_GRID_L3_OUTPUT_CURRENT] = sGrowattModbusReg_t{47, 0, SIZE_16BIT, "GridL3OutputCurrent", 0.1, CURRENT, false, false}; // #18
    Protocol.InputRegisters[P120_GRID_L3_OUTPUT_POWER] = sGrowattModbusReg_t{48, 0, SIZE_32BIT, "GridL3OutputPower", 0.1, VA, false, false};          // #19
    // FRAGMENT 1: END

    // FEAGMENT 2: BEGIN
    Protocol.InputRegisters[P120_ENERGY_TODAY] = sGrowattModbusReg_t{53, 0, SIZE_32BIT, "EnergyToday", 0.1, POWER_KWH, true, false};     // #20
    Protocol.InputRegisters[P120_ENERGY_TOTAL] = sGrowattModbusReg_t{55, 0, SIZE_32BIT, "EnergyTotal", 0.1, POWER_KWH, true, false};     // #21
    Protocol.InputRegisters[P120_WORK_TIME_TOTAL] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "WorkTimeTotal", 0.5, SECONDS, false, false}; // #22

    Protocol.InputRegisters[P120_PV1_ENERGY_TODAY] = sGrowattModbusReg_t{59, 0, SIZE_32BIT, "PV1EnergyToday", 0.1, POWER_KWH, false, false}; // #23
    Protocol.InputRegisters[P120_PV1_ENERGY_TOTAL] = sGrowattModbusReg_t{61, 0, SIZE_32BIT, "PV1EnergyTotal", 0.1, POWER_KWH, false, false}; // #24
    Protocol.InputRegisters[P120_PV2_ENERGY_TODAY] = sGrowattModbusReg_t{63, 0, SIZE_32BIT, "PV2EnergyToday", 0.1, POWER_KWH, false, false}; // #25
    Protocol.InputRegisters[P120_PV2_ENERGY_TOTAL] = sGrowattModbusReg_t{65, 0, SIZE_32BIT, "PV2EnergyTotal", 0.1, POWER_KWH, false, false}; // #26
    Protocol.InputRegisters[P120_PV_ENERGY_TOTAL] = sGrowattModbusReg_t{91, 0, SIZE_32BIT, "PVEnergyTotal", 0.1, POWER_KWH, true, false};    // #27

    Protocol.InputRegisters[P120_INVERTER_TEMPERATURE] = sGrowattModbusReg_t{93, 0, SIZE_16BIT, "InverterTemperature", 0.1, TEMPERATURE, true, true};          // #28
    Protocol.InputRegisters[P120_INVERTER_IPM_TEMPERATURE] = sGrowattModbusReg_t{94, 0, SIZE_16BIT, "InverterIPMTemperature", 0.1, TEMPERATURE, false, false}; // #29
    // FEAGMENT 2: END

    // definition of holding registers
    Protocol.HoldingRegisterCount = 3;
    Protocol.HoldingFragmentCount = 1;
    Protocol.HoldingReadFragments[0] = sGrowattReadFragment_t{0, 4};

    // FRAGMENT 1: BEGIN
    Protocol.HoldingRegisters[P120_OnOff] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "OnOff", 1, NONE, true, false};                         // #1
    Protocol.HoldingRegisters[P120_CMD_MEMORY_STATE] = sGrowattModbusReg_t{2, 0, SIZE_16BIT, "CmdMemoryState", 1, NONE, true, false};     // #2
    Protocol.HoldingRegisters[P120_Active_P_Rate] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "ActivePowerRate", 1, PRECENTAGE, true, false}; // #3
    // FRAGMENT 1: END
}
