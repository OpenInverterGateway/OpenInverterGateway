#include "Arduino.h"

#include "Growatt124.h"


// NOTE: my inverter (SPH4-10KTL3 BH-UP) only manages to read 64 registers in one read!
void init_growatt124(sProtocolDefinition_t &Protocol) {
    // definition of input registers
    Protocol.InputRegisterCount = 52;
    // address, value, size, name, multiplier, unit, frontend, plot
    // FEAGMENT 1: BEGIN
    Protocol.InputRegisters[P124_I_STATUS] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "InverterStatus", 1, NONE, true, false}; // #1
    Protocol.InputRegisters[P124_INPUT_POWER] = sGrowattModbusReg_t{1, 0, SIZE_32BIT, "Input power", 0.1, POWER_W, true, true}; // #2

    Protocol.InputRegisters[P124_PV1_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "PV1 voltage", 0.1, VOLTAGE, false, false}; // #3
    Protocol.InputRegisters[P124_PV1_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "PV1 input current", 0.1, CURRENT, false, false}; // #4
    Protocol.InputRegisters[P124_PV1_POWER] = sGrowattModbusReg_t{5, 0, SIZE_32BIT, "PV1 input power", 0.1, POWER_W, false, false}; // #5
    Protocol.InputRegisters[P124_PV2_VOLTAGE] = sGrowattModbusReg_t{7, 0, SIZE_16BIT, "PV2 voltage", 0.1, VOLTAGE, false, false}; // #6
    Protocol.InputRegisters[P124_PV2_CURRENT] = sGrowattModbusReg_t{8, 0, SIZE_16BIT, "PV2 input current", 0.1, CURRENT, false, false}; // #7
    Protocol.InputRegisters[P124_PV2_POWER] = sGrowattModbusReg_t{9, 0, SIZE_32BIT, "PV2 input power", 0.1, POWER_W, false, false}; // #8

    Protocol.InputRegisters[P124_PAC] = sGrowattModbusReg_t{35, 0, SIZE_32BIT, "Output power", 0.1, POWER_W, true, true}; // #9
    Protocol.InputRegisters[P124_FAC] = sGrowattModbusReg_t{37, 0, SIZE_16BIT, "Grid frequency", 0.01, FREQUENCY, false, false}; // #10

    Protocol.InputRegisters[P124_VAC1] = sGrowattModbusReg_t{38, 0, SIZE_16BIT, "L1 Three phase grid voltage", 0.1, VOLTAGE, false, false}; // #11
    Protocol.InputRegisters[P124_IAC1] = sGrowattModbusReg_t{39, 0, SIZE_16BIT, "L1 Three phase grid output current", 0.1, CURRENT, false, false}; // #12
    Protocol.InputRegisters[P124_PAC1] = sGrowattModbusReg_t{40, 0, SIZE_32BIT, "L1 Three phase grid output power", 0.1, VA, false, false}; // #13
    Protocol.InputRegisters[P124_VAC2] = sGrowattModbusReg_t{42, 0, SIZE_16BIT, "L2 Three phase grid voltage", 0.1, VOLTAGE, false, false}; // #14
    Protocol.InputRegisters[P124_IAC2] = sGrowattModbusReg_t{43, 0, SIZE_16BIT, "L2 Three phase grid output current", 0.1, CURRENT, false, false}; // #15
    Protocol.InputRegisters[P124_PAC2] = sGrowattModbusReg_t{44, 0, SIZE_32BIT, "L2 Three phase grid output power", 0.1, VA, false, false}; // #16
    Protocol.InputRegisters[P124_VAC3] = sGrowattModbusReg_t{46, 0, SIZE_16BIT, "L3 Three phase grid voltage", 0.1, VOLTAGE, false, false}; // #17
    Protocol.InputRegisters[P124_IAC3] = sGrowattModbusReg_t{47, 0, SIZE_16BIT, "L3 Three phase grid output current", 0.1, CURRENT, false, false}; // #18
    Protocol.InputRegisters[P124_PAC3] = sGrowattModbusReg_t{48, 0, SIZE_32BIT, "L3 Three phase grid output power", 0.1, VA, false, false}; // #19
    // FEAGMENT 1: END

    // FEAGMENT 2: BEGIN
    Protocol.InputRegisters[P124_EAC_TODAY] = sGrowattModbusReg_t{53, 0, SIZE_32BIT, "Today generate energy", 0.1, POWER_KWH, true, false}; // #20
    Protocol.InputRegisters[P124_EAC_TOTAL] = sGrowattModbusReg_t{55, 0, SIZE_32BIT, "Total generate energy", 0.1, POWER_KWH, true, false}; // #21
    Protocol.InputRegisters[P124_TIME_TOTAL] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "TWork time total", 0.5, SECONDS, false, false}; // #22

    Protocol.InputRegisters[P124_EPV1_TODAY] = sGrowattModbusReg_t{59, 0, SIZE_32BIT, "PV1 Energy today", 0.1, POWER_KWH, false, false}; // #23
    Protocol.InputRegisters[P124_EPV1_TOTAL] = sGrowattModbusReg_t{61, 0, SIZE_32BIT, "PV1 Energy total", 0.1, POWER_KWH, false, false}; // #24
    Protocol.InputRegisters[P124_EPV2_TODAY] = sGrowattModbusReg_t{63, 0, SIZE_32BIT, "PV2 Energy today", 0.1, POWER_KWH, false, false}; // #25
    Protocol.InputRegisters[P124_EPV2_TOTAL] = sGrowattModbusReg_t{65, 0, SIZE_32BIT, "PV2 Energy total", 0.1, POWER_KWH, false, false}; // #26
    Protocol.InputRegisters[P124_EPV_TOTAL] = sGrowattModbusReg_t{91, 0, SIZE_32BIT, "PV Energy total", 0.1, POWER_KWH, false, false}; // #27

    Protocol.InputRegisters[P124_TEMP1] = sGrowattModbusReg_t{93, 0, SIZE_16BIT, "Inverter temperature", 0.1, TEMPERATURE, true, true}; // #28
    Protocol.InputRegisters[P124_TEMP2] = sGrowattModbusReg_t{94, 0, SIZE_16BIT, "Temperature inside IPM", 0.1, TEMPERATURE, false, false}; // #29
    Protocol.InputRegisters[P124_TEMP3] = sGrowattModbusReg_t{95, 0, SIZE_16BIT, "Boost temperature", 0.1, TEMPERATURE, false, false}; // #30
    // FEAGMENT 2: END

    // FEAGMENT 3: BEGIN
    Protocol.InputRegisters[P124_PDISCHARGE] = sGrowattModbusReg_t{1009, 0, SIZE_32BIT, "Discharge power", 0.1, POWER_W, true, true}; // #31
    Protocol.InputRegisters[P124_PCHARGE] = sGrowattModbusReg_t{1011, 0, SIZE_32BIT, "Charge power", 0.1, POWER_W, true, true}; // #32
    Protocol.InputRegisters[P124_VBAT] = sGrowattModbusReg_t{1013, 0, SIZE_16BIT, "Battery voltage", 0.1, VOLTAGE, false, false}; // #33
    Protocol.InputRegisters[P124_SOC] = sGrowattModbusReg_t{1014, 0, SIZE_16BIT, "SOC", 1, PRECENTAGE, true, true}; // #34
    Protocol.InputRegisters[P124_PAC_TO_USER] = sGrowattModbusReg_t{1015, 0, SIZE_32BIT, "AC power to user", 0.1, POWER_W, false, false}; // #35
    Protocol.InputRegisters[P124_PAC_TO_USER_TOTAL] = sGrowattModbusReg_t{1021, 0, SIZE_32BIT, "AC power to user total", 0.1, POWER_W, false, false}; // #36
    Protocol.InputRegisters[P124_PAC_TO_GRID] = sGrowattModbusReg_t{1023, 0, SIZE_32BIT, "AC power to grid", 0.1, POWER_W, false, false}; // #37
    Protocol.InputRegisters[P124_PAC_TO_GRID_TOTAL] = sGrowattModbusReg_t{1029, 0, SIZE_32BIT, "AC power to grid total", 0.1, POWER_W, false, false}; // #38
    Protocol.InputRegisters[P124_PLOCAL_LOAD] = sGrowattModbusReg_t{1031, 0, SIZE_32BIT, "INV power to local load", 0.1, POWER_W, false, false}; // #39
    Protocol.InputRegisters[P124_PLOCAL_LOAD_TOTAL] = sGrowattModbusReg_t{1037, 0, SIZE_32BIT, "INV power to local load total", 0.1, POWER_W, true, false}; // #40
    Protocol.InputRegisters[P124_BATTERY_TEMPERATURE] = sGrowattModbusReg_t{1040, 0, SIZE_16BIT, "Battery Temperature", 0.1, TEMPERATURE, true, true}; // #41
    Protocol.InputRegisters[P124_BATTERY_STATE] = sGrowattModbusReg_t{1041, 0, SIZE_16BIT, "Battery State", 1, NONE, true, false}; // #42

    Protocol.InputRegisters[P124_ETOUSER_TODAY] = sGrowattModbusReg_t{1044, 0, SIZE_32BIT, "Energy to user today", 0.1, POWER_KWH, true, false}; // #43
    Protocol.InputRegisters[P124_ETOUSER_TOTAL] = sGrowattModbusReg_t{1046, 0, SIZE_32BIT, "Energy to user total", 0.1, POWER_KWH, true, false}; // #44
    Protocol.InputRegisters[P124_ETOGRID_TODAY] = sGrowattModbusReg_t{1048, 0, SIZE_32BIT, "Energy to grid today", 0.1, POWER_KWH, true, false}; // #45
    Protocol.InputRegisters[P124_ETOGRID_TOTAL] = sGrowattModbusReg_t{1050, 0, SIZE_32BIT, "Energy to grid total", 0.1, POWER_KWH, true, false}; // #46
    Protocol.InputRegisters[P124_EDISCHARGE_TODAY] = sGrowattModbusReg_t{1052, 0, SIZE_32BIT, "Discharge energy today", 0.1, POWER_KWH, true, false}; // #47
    Protocol.InputRegisters[P124_EDISCHARGE_TOTAL] = sGrowattModbusReg_t{1054, 0, SIZE_32BIT, "Discharge energy total", 0.1, POWER_KWH, true, false}; // #48
    Protocol.InputRegisters[P124_ECHARGE_TODAY] = sGrowattModbusReg_t{1056, 0, SIZE_32BIT, "Charge energy today", 0.1, POWER_KWH, true, false}; // #49
    Protocol.InputRegisters[P124_ECHARGE_TOTAL] = sGrowattModbusReg_t{1058, 0, SIZE_32BIT, "Charge energy total", 0.1, POWER_KWH, true, false}; // #50
    Protocol.InputRegisters[P124_ETOLOCALLOAD_TODAY] = sGrowattModbusReg_t{1060, 0, SIZE_32BIT, "Local load energy today", 0.1, POWER_KWH, true, false}; // #51
    Protocol.InputRegisters[P124_ETOLOCALLOAD_TOTAL] = sGrowattModbusReg_t{1062, 0, SIZE_32BIT, "Local load energy total", 0.1, POWER_KWH, true, false}; // #52
    // FEAGMENT 3: END

    Protocol.InputFragmentCount = 3;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 49};
    Protocol.InputReadFragments[1] = sGrowattReadFragment_t{53, 43};
    Protocol.InputReadFragments[2] = sGrowattReadFragment_t{1009, 55};

    Protocol.HoldingRegisterCount = 0;
    Protocol.HoldingFragmentCount = 0;
}