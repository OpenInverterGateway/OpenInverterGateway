#include "Arduino.h"

#include "Growatt305.h"

void init_growatt305(sProtocolDefinition_t& Protocol)
{
    // definition of input registers
    Protocol.InputRegisterCount = 12;
    // address, value, size, name, multiplier, unit, frontend, plot
    // FEAGMENT 1: BEGIN
    Protocol.InputRegisters[P305_I_STATUS] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "InverterStatus", 1, NONE, true, false};                 // #1
    Protocol.InputRegisters[P305_DC_POWER] = sGrowattModbusReg_t{1, 0, SIZE_32BIT, "DcPower", 0.1, POWER_W, true, true};                    // #2
    Protocol.InputRegisters[P305_DC_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "DcVoltage", 0.1, VOLTAGE, true, false};               // #3
    Protocol.InputRegisters[P305_DC_INPUT_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "DcInputCurrent", 0.1, CURRENT, true, false};    // #4
    Protocol.InputRegisters[P305_AC_FREQUENCY] = sGrowattModbusReg_t{13, 0, SIZE_16BIT, "AcFrequency", 0.01, FREQUENCY, true, false};       // #5
    Protocol.InputRegisters[P305_AC_VOLTAGE] = sGrowattModbusReg_t{14, 0, SIZE_16BIT, "AcVoltage", 0.1, VOLTAGE, true, false};              // #6
    Protocol.InputRegisters[P305_AC_OUTPUT_CURRENT] = sGrowattModbusReg_t{15, 0, SIZE_16BIT, "AcOutputCurrent", 0.1, CURRENT, true, false}; // #7
    Protocol.InputRegisters[P305_AC_POWER] = sGrowattModbusReg_t{16, 0, SIZE_32BIT, "AcPower", 0.1, POWER_W, true, true};                   // #8
    Protocol.InputRegisters[P305_ENERGY_TODAY] = sGrowattModbusReg_t{26, 0, SIZE_32BIT, "EnergyToday", 0.1, POWER_KWH, true, false};        // #9
    Protocol.InputRegisters[P305_ENERGY_TOTAL] = sGrowattModbusReg_t{28, 0, SIZE_32BIT, "EnergyTotal", 0.1, POWER_KWH, true, false};        // #10
    Protocol.InputRegisters[P305_OPERATING_TIME] = sGrowattModbusReg_t{30, 0, SIZE_32BIT, "OperatingTime", 0.5, SECONDS, true, false};      // #11
    Protocol.InputRegisters[P305_TEMPERATURE] = sGrowattModbusReg_t{32, 0, SIZE_16BIT, "Temperature", 0.1, TEMPERATURE, true, false};       // #12

    Protocol.InputFragmentCount = 1;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 33};

    Protocol.HoldingRegisterCount = 0;
    Protocol.HoldingFragmentCount = 0;
}
