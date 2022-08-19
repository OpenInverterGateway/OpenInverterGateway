#include "Arduino.h"

#include "Growatt124.h"

sProtocolDefinition_t init_growatt124() {
    // definition of input registers
    uint16_t InputRegisterCount = 2;
    // address, value, size, name, multiplier, unit, frontend
    sGrowattModbusReg_t InputRegisters[29];
    InputRegisters[I_STATUS] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "InverterStatus", 1, INVERTER_STATUS, true}; // #1
    InputRegisters[INPUT_POWER] = sGrowattModbusReg_t{1, 0, SIZE_32BIT, "Input power", 0.1, POWER_W, true}; // #2

    // InputRegisters[PV1_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "PV1 voltage", 0.1, VOLTAGE, true}; // #3
    // InputRegisters[PV1_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "PV1 input current", 0.1, CURRENT, true}; // #4
    // InputRegisters[PV1_POWER] = sGrowattModbusReg_t{5, 0, SIZE_32BIT, "PV1 input power", 0.1, POWER_W, true}; // #5
    // InputRegisters[PV2_VOLTAGE] = sGrowattModbusReg_t{7, 0, SIZE_16BIT, "PV2 voltage", 0.1, VOLTAGE, true}; // #6
    // InputRegisters[PV2_CURRENT] = sGrowattModbusReg_t{8, 0, SIZE_16BIT, "PV2 input current", 0.1, CURRENT, true}; // #7
    // InputRegisters[PV2_POWER] = sGrowattModbusReg_t{9, 0, SIZE_32BIT, "PV2 input power", 0.1, POWER_W, true}; // #8

    // InputRegisters[PAC] = sGrowattModbusReg_t{35, 0, SIZE_32BIT, "Output power", 0.1, POWER_W, true}; // #9
    // InputRegisters[FAC] = sGrowattModbusReg_t{37, 0, SIZE_16BIT, "Grid frequency", 0.1, FREQUENCY, true}; // #10

    // InputRegisters[VAC1] = sGrowattModbusReg_t{38, 0, SIZE_16BIT, "L1 Three phase grid voltage", 0.1, VOLTAGE, true}; // #11
    // InputRegisters[IAC1] = sGrowattModbusReg_t{39, 0, SIZE_16BIT, "L1 Three phase grid output current", 0.1, CURRENT, true}; // #12
    // InputRegisters[PAC1] = sGrowattModbusReg_t{40, 0, SIZE_32BIT, "L1 Three phase grid output power", 0.1, POWER_W, true}; // #13
    // InputRegisters[VAC2] = sGrowattModbusReg_t{42, 0, SIZE_16BIT, "L2 Three phase grid voltage", 0.1, VOLTAGE, true}; // #14
    // InputRegisters[IAC2] = sGrowattModbusReg_t{43, 0, SIZE_16BIT, "L2 Three phase grid output current", 0.1, CURRENT, true}; // #15
    // InputRegisters[PAC2] = sGrowattModbusReg_t{44, 0, SIZE_32BIT, "L2 Three phase grid output power", 0.1, POWER_W, true}; // #16
    // InputRegisters[VAC3] = sGrowattModbusReg_t{46, 0, SIZE_16BIT, "L3 Three phase grid voltage", 0.1, VOLTAGE, true}; // #17
    // InputRegisters[IAC3] = sGrowattModbusReg_t{47, 0, SIZE_16BIT, "L3 Three phase grid output current", 0.1, CURRENT, true}; // #18
    // InputRegisters[PAC3] = sGrowattModbusReg_t{48, 0, SIZE_32BIT, "L3 Three phase grid output power", 0.1, POWER_W, true}; // #19

    // InputRegisters[EAC_TODAY] = sGrowattModbusReg_t{53, 0, SIZE_32BIT, "Today generate energy", 0.1, POWER_KWH, true}; // #20
    // InputRegisters[EAC_TOTAL] = sGrowattModbusReg_t{55, 0, SIZE_32BIT, "Total generate energy", 0.1, POWER_KWH, true}; // #21
    // InputRegisters[TIME_TOTAL] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "TWork time total", 0.5, SECONDS, true}; // #22

    // InputRegisters[EPV1_TODAY] = sGrowattModbusReg_t{59, 0, SIZE_32BIT, "PV1 Energy today", 0.1, POWER_KWH, true}; // #23
    // InputRegisters[EPV1_TOTAL] = sGrowattModbusReg_t{61, 0, SIZE_32BIT, "PV1 Energy total", 0.1, POWER_KWH, true}; // #24
    // InputRegisters[EPV2_TODAY] = sGrowattModbusReg_t{63, 0, SIZE_32BIT, "PV2 Energy today", 0.1, POWER_KWH, true}; // #25
    // InputRegisters[EPV2_TOTAL] = sGrowattModbusReg_t{65, 0, SIZE_32BIT, "PV2 Energy total", 0.1, POWER_KWH, true}; // #26

    // InputRegisters[TEMP1] = sGrowattModbusReg_t{93, 0, SIZE_16BIT, "Inverter temperature", 0.1, TEMPERATURE, true}; // #27
    // InputRegisters[TEMP2] = sGrowattModbusReg_t{94, 0, SIZE_16BIT, "Temperature inside IPM", 0.1, TEMPERATURE, true}; // #28
    // InputRegisters[TEMP3] = sGrowattModbusReg_t{95, 0, SIZE_16BIT, "Boost temperature", 0.1, TEMPERATURE, true}; // #29

    uint8_t InputFragmentCount = 1;
    sGrowattReadFragment_t InputReadFragments[1];
    InputReadFragments[0] = sGrowattReadFragment_t{0, 95};

    uint16_t HoldingRegisterCount = 0;
    sGrowattModbusReg_t HoldingRegisters[0];
    uint8_t HoldingFragmentCount = 0;
    sGrowattReadFragment_t HoldingReadFragments[0];

    return {
        InputRegisterCount, InputFragmentCount, // input registers count
        HoldingRegisterCount, HoldingFragmentCount, // holding registers count
        InputRegisters, HoldingRegisters, // register definitions
        InputReadFragments, HoldingReadFragments // read fragment definitions
    };
}