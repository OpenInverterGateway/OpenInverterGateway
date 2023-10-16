#include "Arduino.h"

#include "Config.h"
#include "Growatt.h"
#include "GrowattTLXH.h"

void init_growattTLXH(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  Protocol.InputRegisterCount = 49;
  Protocol.InputRegisters[TLXH_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};  // #1
  Protocol.InputRegisters[TLXH_INPUT_POWER] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("InputPower"), 0.1, 0.1, POWER_W, true, true};  // #2
  Protocol.InputRegisters[TLXH_PV1_VOLTAGE] =
      sGrowattModbusReg_t{3,       0,     SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1,
                          VOLTAGE, false, false};  // #3
  Protocol.InputRegisters[TLXH_PV1_INPUT_CURRENT] = sGrowattModbusReg_t{
      4,       0,     SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #4
  Protocol.InputRegisters[TLXH_PV1_POWER] = sGrowattModbusReg_t{
      5,       0,     SIZE_32BIT, F("PV1InputPower"), 0.1, 0.1,
      POWER_W, false, false};  // #5
  Protocol.InputRegisters[TLXH_PV2_VOLTAGE] =
      sGrowattModbusReg_t{7,       0,     SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1,
                          VOLTAGE, false, false};  // #6
  Protocol.InputRegisters[TLXH_PV2_INPUT_CURRENT] = sGrowattModbusReg_t{
      8,       0,     SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #7
  Protocol.InputRegisters[TLXH_PV2_POWER] = sGrowattModbusReg_t{
      9,       0,     SIZE_32BIT, F("PV2InputPower"), 0.1, 0.1,
      POWER_W, false, false};  // #8
  Protocol.InputRegisters[TLXH_PAC] =
      sGrowattModbusReg_t{35,      0,    SIZE_32BIT, F("OutputPower"), 0.1, 0.1,
                          POWER_W, true, true};  // #9
  Protocol.InputRegisters[TLXH_FAC] = sGrowattModbusReg_t{
      37,        0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
      FREQUENCY, false, false};  // #10
  Protocol.InputRegisters[TLXH_VAC1] = sGrowattModbusReg_t{
      38,      0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #11
  Protocol.InputRegisters[TLXH_IAC1] = sGrowattModbusReg_t{
      39,      0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #12
  Protocol.InputRegisters[TLXH_PAC1] = sGrowattModbusReg_t{
      40, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #13
  Protocol.InputRegisters[TLXH_VAC2] = sGrowattModbusReg_t{
      42,      0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #14
  Protocol.InputRegisters[TLXH_IAC2] = sGrowattModbusReg_t{
      43,      0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #15
  Protocol.InputRegisters[TLXH_PAC2] = sGrowattModbusReg_t{
      44, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #16
  Protocol.InputRegisters[TLXH_VAC3] = sGrowattModbusReg_t{
      46,      0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};  // #17
  Protocol.InputRegisters[TLXH_IAC3] = sGrowattModbusReg_t{
      47,      0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};  // #18
  Protocol.InputRegisters[TLXH_PAC3] = sGrowattModbusReg_t{
      48, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};  // #19
  // FEAGMENT 1: END

  // FEAGMENT 2: BEGIN
  Protocol.InputRegisters[TLXH_EAC_TODAY] = sGrowattModbusReg_t{
      53,        0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};  // #20
  Protocol.InputRegisters[TLXH_EAC_TOTAL] = sGrowattModbusReg_t{
      55,        0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};  // #21
  Protocol.InputRegisters[TLXH_TIME_TOTAL] = sGrowattModbusReg_t{
      57,      0,     SIZE_32BIT, F("TWorkTimeTotal"), 0.5, 0.5,
      SECONDS, false, false};  // #22
  Protocol.InputRegisters[TLXH_EPV1_TODAY] = sGrowattModbusReg_t{
      59,        0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};  // #23
  Protocol.InputRegisters[TLXH_EPV1_TOTAL] = sGrowattModbusReg_t{
      61,        0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #24
  Protocol.InputRegisters[TLXH_EPV2_TODAY] = sGrowattModbusReg_t{
      63,        0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};  // #25
  Protocol.InputRegisters[TLXH_EPV2_TOTAL] = sGrowattModbusReg_t{
      65,        0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #26
  Protocol.InputRegisters[TLXH_EPV_TOTAL] = sGrowattModbusReg_t{
      91,        0,     SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};  // #27
  Protocol.InputRegisters[TLXH_TEMP1] = sGrowattModbusReg_t{
      93,          0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
      TEMPERATURE, true, true};  // #28
  Protocol.InputRegisters[TLXH_TEMP2] = sGrowattModbusReg_t{
      94,          0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
      TEMPERATURE, false, false};  // #29
  Protocol.InputRegisters[TLXH_TEMP3] = sGrowattModbusReg_t{
      95,          0,     SIZE_16BIT, F("BoostTemperature"), 0.1, 0.1,
      TEMPERATURE, false, false};  // #30
  // FEAGMENT 2: END

  // FEAGMENT 3: BEGIN
  Protocol.InputRegisters[TLXH_PAC_TO_USER_TOTAL] = sGrowattModbusReg_t{
      3041,    0,     SIZE_32BIT, F("ACPowerToUserTotal"), 0.1, 0.1,
      POWER_W, false, false};  // #31
  Protocol.InputRegisters[TLXH_PAC_TO_GRID_TOTAL] = sGrowattModbusReg_t{
      3043,    0,     SIZE_32BIT, F("ACPowerToGridTotal"), 0.1, 0.1,
      POWER_W, false, false};  // #32
  Protocol.InputRegisters[TLXH_PLOCAL_LOAD_TOTAL] = sGrowattModbusReg_t{
      3045,    0,    SIZE_32BIT, F("INVPowerToLocalLoadTotal"), 0.1, 0.1,
      POWER_W, true, false};  // #33
  Protocol.InputRegisters[TLXH_ETOUSER_TODAY] = sGrowattModbusReg_t{
      3067,      0,    SIZE_32BIT, F("EnergyToUserToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #34
  Protocol.InputRegisters[TLXH_ETOUSER_TOTAL] = sGrowattModbusReg_t{
      3069,      0,    SIZE_32BIT, F("EnergyToUserTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #35
  Protocol.InputRegisters[TLXH_ETOGRID_TODAY] = sGrowattModbusReg_t{
      3071,      0,    SIZE_32BIT, F("EnergyToGridToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #36
  Protocol.InputRegisters[TLXH_ETOGRID_TOTAL] = sGrowattModbusReg_t{
      3073,      0,    SIZE_32BIT, F("EnergyToGridTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #37
  Protocol.InputRegisters[TLXH_ETOLOCALLOAD_TODAY] = sGrowattModbusReg_t{
      3075,      0,    SIZE_32BIT, F("LocalLoadEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #38
  Protocol.InputRegisters[TLXH_ETOLOCALLOAD_TOTAL] = sGrowattModbusReg_t{
      3077,      0,    SIZE_32BIT, F("LocalLoadEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #39
  // FEAGMENT 3: END

  // FEAGMENT 4: BEGIN
  Protocol.InputRegisters[TLXH_EDISCHARGE_TODAY] = sGrowattModbusReg_t{
      3125,      0,    SIZE_32BIT, F("DischargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #40
  Protocol.InputRegisters[TLXH_EDISCHARGE_TOTAL] = sGrowattModbusReg_t{
      3127,      0,    SIZE_32BIT, F("DischargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #41
  Protocol.InputRegisters[TLXH_ECHARGE_TODAY] = sGrowattModbusReg_t{
      3129,      0,    SIZE_32BIT, F("ChargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #42
  Protocol.InputRegisters[TLXH_ECHARGE_TOTAL] = sGrowattModbusReg_t{
      3131,      0,    SIZE_32BIT, F("ChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #43
  Protocol.InputRegisters[TLXH_BATTERY_STATE] = sGrowattModbusReg_t{
      3166, 0, SIZE_16BIT, F("BatteryState"), 1, 1, NONE, true, false};  // #44
  Protocol.InputRegisters[TLXH_VBAT] = sGrowattModbusReg_t{
      3169,    0,     SIZE_16BIT, F("BatteryVoltage"), 0.01, 0.01,
      VOLTAGE, false, false};  // #45
  Protocol.InputRegisters[TLXH_SOC] = sGrowattModbusReg_t{
      3171, 0, SIZE_16BIT, F("SOC"), 1, 1, PERCENTAGE, true, true};  // #46
  Protocol.InputRegisters[TLXH_BATTERY_TEMPERATURE] = sGrowattModbusReg_t{
      3176,        0,    SIZE_16BIT, F("BatteryTemperature"), 0.1, 0.1,
      TEMPERATURE, true, true};  // #47
  Protocol.InputRegisters[TLXH_PDISCHARGE] = sGrowattModbusReg_t{
      3178,    0,    SIZE_32BIT, F("DischargePower"), 0.1, 0.1,
      POWER_W, true, true};  // #48
  Protocol.InputRegisters[TLXH_PCHARGE] =
      sGrowattModbusReg_t{3180,    0,    SIZE_32BIT, F("ChargePower"), 0.1, 0.1,
                          POWER_W, true, true};  // #49
  // FEAGMENT 4: END

  Protocol.InputFragmentCount = 4;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 50};
  Protocol.InputReadFragments[1] = sGrowattReadFragment_t{53, 43};
  Protocol.InputReadFragments[2] = sGrowattReadFragment_t{3041, 38};
  Protocol.InputReadFragments[3] = sGrowattReadFragment_t{3125, 57};

  Protocol.HoldingRegisterCount = 0;
  Protocol.HoldingFragmentCount = 0;
}