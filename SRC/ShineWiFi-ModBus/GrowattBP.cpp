#include "Arduino.h"

#include "Growatt.h"
#include "GrowattBP.h"

void init_growattBP(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  // definition of input registers
  Protocol.InputRegisterCount = 32;

  // address, value, size, name, multiplier, resolution, unit, frontend, plot

  // general
  Protocol.InputRegisters[BP_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};  // #1
  Protocol.InputRegisters[BP_INPUT_POWER] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("InputPower"), 0.1, 0.1, POWER_W, true, true};  // #2
  Protocol.InputRegisters[BP_OUTPUT_POWER] = sGrowattModbusReg_t{
      35,      0,    SIZE_32BIT_S, F("OutputPower"), 0.1, 0.1,
      POWER_W, true, true};  // #3

  // input 1
  Protocol.InputRegisters[BP_PV1_VOLTAGE] =
      sGrowattModbusReg_t{3,       0,     SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1,
                          VOLTAGE, false, false};  // #4
  Protocol.InputRegisters[BP_PV1_CURRENT] = sGrowattModbusReg_t{
      4,       0,     SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #5
  Protocol.InputRegisters[BP_PV1_POWER] = sGrowattModbusReg_t{
      5,       0,     SIZE_32BIT, F("PV1InputPower"), 0.1, 0.1,
      POWER_W, false, false};  // #6

  // input 2
  Protocol.InputRegisters[BP_PV2_VOLTAGE] =
      sGrowattModbusReg_t{7,       0,     SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1,
                          VOLTAGE, false, false};  // #7
  Protocol.InputRegisters[BP_PV2_CURRENT] = sGrowattModbusReg_t{
      8,       0,     SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #8
  Protocol.InputRegisters[BP_PV2_POWER] = sGrowattModbusReg_t{
      9,       0,     SIZE_32BIT, F("PV2InputPower"), 0.1, 0.1,
      POWER_W, false, false};  // #9

  // grid and phase
  Protocol.InputRegisters[BP_GRID_FREQUENCY] = sGrowattModbusReg_t{
      37,        0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
      FREQUENCY, false, false};  // #10
  Protocol.InputRegisters[BP_AC1_VOLTAGE] = sGrowattModbusReg_t{
      38,      0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #11
  Protocol.InputRegisters[BP_AC1_CURRENT] = sGrowattModbusReg_t{
      39,      0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #12
  Protocol.InputRegisters[BP_AC1_POWER] = sGrowattModbusReg_t{
      40, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #13
  Protocol.InputRegisters[BP_AC2_VOLTAGE] = sGrowattModbusReg_t{
      42,      0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #14
  Protocol.InputRegisters[BP_AC2_CURRENT] = sGrowattModbusReg_t{
      43,      0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #15
  Protocol.InputRegisters[BP_AC2_POWER] = sGrowattModbusReg_t{
      44, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #16
  Protocol.InputRegisters[BP_AC3_VOLTAGE] = sGrowattModbusReg_t{
      46,      0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #17
  Protocol.InputRegisters[BP_AC3_CURRENT] = sGrowattModbusReg_t{
      47,      0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #18
  Protocol.InputRegisters[BP_AC3_POWER] = sGrowattModbusReg_t{
      48, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #19

  // statistics
  Protocol.InputRegisters[BP_EAC_TODAY] = sGrowattModbusReg_t{
      53,        0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};  // #20
  Protocol.InputRegisters[BP_EAC_TOTAL] = sGrowattModbusReg_t{
      55,        0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};  // #21
  Protocol.InputRegisters[BP_TIME_TOTAL] = sGrowattModbusReg_t{
      57,      0,     SIZE_32BIT, F("TWorkTimeTotal"), 0.5, 1,
      SECONDS, false, false};  // #22

  Protocol.InputRegisters[BP_EPV1_TODAY] = sGrowattModbusReg_t{
      59,        0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};  // #23
  Protocol.InputRegisters[BP_EPV1_TOTAL] = sGrowattModbusReg_t{
      61,        0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #24
  Protocol.InputRegisters[BP_EPV2_TODAY] = sGrowattModbusReg_t{
      63,        0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};  // #25
  Protocol.InputRegisters[BP_EPV2_TOTAL] = sGrowattModbusReg_t{
      65,        0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #26
  Protocol.InputRegisters[BP_EPV_TOTAL] = sGrowattModbusReg_t{
      91,        0,     SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #27

  // temperature
  Protocol.InputRegisters[BP_TEMP1] = sGrowattModbusReg_t{
      93,          0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
      TEMPERATURE, true, true};  // #28
  Protocol.InputRegisters[BP_TEMP2] = sGrowattModbusReg_t{
      94,          0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
      TEMPERATURE, false, false};  // #29

  // battery
  Protocol.InputRegisters[BP_BAT_PERCENTAGE] = sGrowattModbusReg_t{
      4014,       0,    SIZE_16BIT, F("BatteryPercentage"), 1, 1,
      PERCENTAGE, true, true};  // #30
  Protocol.InputRegisters[BP_BAT_CHARGE_POWER] = sGrowattModbusReg_t{
      4023,    0,    SIZE_32BIT, F("BatteryCharge"), 0.1, 0.1,
      POWER_W, true, true};  // #31
  Protocol.InputRegisters[BP_BAT_DISCHARGE_POWER] = sGrowattModbusReg_t{
      4021,    0,    SIZE_32BIT, F("BatteryDischarge"), 0.1, 0.1,
      POWER_W, true, true};  // #32

  Protocol.InputFragmentCount = 3;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 50};
  Protocol.InputReadFragments[1] = sGrowattReadFragment_t{50, 50};
  Protocol.InputReadFragments[2] = sGrowattReadFragment_t{4014, 20};

  Protocol.HoldingRegisterCount = 0;
  Protocol.HoldingFragmentCount = 0;
}
