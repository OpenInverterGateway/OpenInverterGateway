#include "Arduino.h"

#include "Config.h"
#include "Growatt.h"
#include "Growatt124.h"
#include "GrowattTLXH.h"
#include <TLog.h>

std::tuple<bool, String> setBDCDischargePowerRate(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  uint16_t value = req["value"].as<uint16_t>();

  if (!((value >= 0 && value <= 100))) {
    return std::make_tuple(false, "'value' field not in range");
  }

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(3036, value)) {
    return std::make_tuple(false, "Failed to write BDCDischargePowerRate");
  }
#endif

  return std::make_tuple(true, "Successfully updated BDCDischargePowerRate");
};

std::tuple<bool, String> setBDCDischargeStopSOC(const JsonDocument& req,
                                                JsonDocument& res,
                                                Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  uint16_t value = req["value"].as<uint16_t>();

  if (!((value >= 0 && value <= 100))) {
    return std::make_tuple(false, "'value' field not in range");
  }

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(3037, value)) {
    return std::make_tuple(false, "Failed to write BDCDischargeStopSOC");
  }
#endif

  return std::make_tuple(true, "Successfully updated BDCDischargeStopSOC");
};

std::tuple<bool, String> setBDCChargePowerRate(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  uint16_t value = req["value"].as<uint16_t>();

  if (!((value >= 0 && value <= 100))) {
    return std::make_tuple(false, "'value' field not in range");
  }

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(3047, value)) {
    return std::make_tuple(false, "Failed to write BDCChargePowerRate");
  }
#endif

  return std::make_tuple(true, "Successfully updated BDCChargePowerRate");
};

std::tuple<bool, String> setBDCChargeStopSOC(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  uint16_t value = req["value"].as<uint16_t>();

  if (!((value >= 0 && value <= 100))) {
    return std::make_tuple(false, "'value' field not in range");
  }

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(3048, value)) {
    return std::make_tuple(false, "Failed to write BDCChargeStopSOC");
  }
#endif

  return std::make_tuple(true, "Successfully updated BDCChargeStopSOC");
};

std::tuple<bool, String> setBDCACChargeEnabled(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(3049, value)) {
    return std::make_tuple(false, "Failed to write AC charge enabled");
  }
#endif

  return std::make_tuple(true, "Successfully updated BDCACChargeEnabled");
}

std::tuple<bool, String> setPriority(const JsonDocument& req, JsonDocument& res,
                                     Growatt& inverter) {
  if (!req.containsKey("mode")) {
    return std::make_tuple(false, "'mode' field is required");
  }
  uint16_t mode = req["mode"].as<uint16_t>();

  if (mode > 2) {
    return std::make_tuple(false,
                           "Invalid priority mode! Select either 0 (load "
                           "first), 1 (battery first) or 2 (grid first)");
  }

  uint16_t mode_raw[2] = {0};
  if (mode == 1) {
    mode_raw[0] = 40960;
    mode_raw[1] = 5947;
  }
  if (mode == 2) {
    mode_raw[0] = 49152;
    mode_raw[1] = 5947;
  }

  if (!inverter.WriteHoldingRegFrag(3038, 2, mode_raw)) {
    return std::make_tuple(false, "Failed to set priority mode");
  }

  return std::make_tuple(true, "success");
}

// TODO: add setters and getters for timeslots.

void init_growattTLXH(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  // definition of input registers
  Protocol.InputRegisterCount = P3000_INPUT_REGISTER_COUNT;
  // address, value, size, name, multiplier, resolution, unit, frontend, plot

  // FRAGMENT 1: BEGIN
  Protocol.InputRegisters[P3000_INVERTER_STATUS] = sGrowattModbusReg_t{
      3000, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_INVERTER_RUNSTATE] = sGrowattModbusReg_t{
      3000, 0, SIZE_16BIT, F("InverterRunState"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P3000_PPV] = sGrowattModbusReg_t{
      3001, 0, SIZE_32BIT, F("PVTotalPower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P3000_VPV1] = sGrowattModbusReg_t{
      3003, 0, SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IPV1] = sGrowattModbusReg_t{
      // TODO: real reg names
      3004,    0,     SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PPV1] = sGrowattModbusReg_t{
      3005, 0, SIZE_32BIT, F("PV1Power"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P3000_VPV2] = sGrowattModbusReg_t{
      3007, 0, SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IPV2] = sGrowattModbusReg_t{
      3008,    0,     SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PPV2] = sGrowattModbusReg_t{
      3009, 0, SIZE_32BIT, F("PV2Power"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P3000_VPV3] = sGrowattModbusReg_t{
      3011, 0, SIZE_16BIT, F("PV3Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IPV3] = sGrowattModbusReg_t{
      3012,    0,     SIZE_16BIT, F("PV3InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PPV3] = sGrowattModbusReg_t{
      3013, 0, SIZE_32BIT, F("PV3Power"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P3000_VPV4] = sGrowattModbusReg_t{
      3015, 0, SIZE_16BIT, F("PV4Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IPV4] = sGrowattModbusReg_t{
      3016,    0,     SIZE_16BIT, F("PV4InputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PPV4] = sGrowattModbusReg_t{
      3017, 0, SIZE_32BIT, F("PV4Power"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P3000_PSYS] = sGrowattModbusReg_t{
      3019,    0,     SIZE_32BIT_S, F("SystemOutputPower"), 0.1, 0.1,
      POWER_W, false, false};
  Protocol.InputRegisters[P3000_QAC] = sGrowattModbusReg_t{
      3021,           0,    SIZE_32BIT_S, F("ReactivePower"), 0.1, 0.1,
      POWER_REACTIVE, true, true};
  Protocol.InputRegisters[P3000_PAC] = sGrowattModbusReg_t{
      3023, 0, SIZE_32BIT_S, F("OutputPower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P3000_FAC] = sGrowattModbusReg_t{
      3025,      0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
      FREQUENCY, false, false};
  Protocol.InputRegisters[P3000_VAC1] = sGrowattModbusReg_t{
      3026,    0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IAC1] = sGrowattModbusReg_t{
      3027,    0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PAC1] = sGrowattModbusReg_t{
      3028, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA,   false, false};
  Protocol.InputRegisters[P3000_VAC2] = sGrowattModbusReg_t{
      3030,    0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IAC2] = sGrowattModbusReg_t{
      3031,    0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PAC2] = sGrowattModbusReg_t{
      3032, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA,   false, false};
  Protocol.InputRegisters[P3000_VAC3] = sGrowattModbusReg_t{
      3034,    0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IAC3] = sGrowattModbusReg_t{
      3035,    0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_PAC3] = sGrowattModbusReg_t{
      3036, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA,   false, false};
  Protocol.InputRegisters[P3000_VAC_RS] = sGrowattModbusReg_t{
      3038,    0,     SIZE_16BIT, F("RSThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_VAC_ST] = sGrowattModbusReg_t{
      3039,    0,     SIZE_16BIT, F("STThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_VAC_TR] = sGrowattModbusReg_t{
      3040,    0,     SIZE_16BIT, F("TRThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_PTOUSER_TOTAL] = sGrowattModbusReg_t{
      3041,    0,    SIZE_32BIT_S, F("TotalForwardPower"), 0.1, 0.1,
      POWER_W, true, true};
  Protocol.InputRegisters[P3000_PTOGRID_TOTAL] = sGrowattModbusReg_t{
      3043,    0,    SIZE_32BIT_S, F("TotalReversePower"), 0.1, 0.1,
      POWER_W, true, true};
  Protocol.InputRegisters[P3000_PTOLOAD_TOTAL] = sGrowattModbusReg_t{
      3045,    0,    SIZE_32BIT_S, F("TotalLoadPower"), 0.1, 0.1,
      POWER_W, true, true};
  Protocol.InputRegisters[P3000_TIME_TOTAL] = sGrowattModbusReg_t{
      3047, 0, SIZE_32BIT, F("WorkTimeTotal"), 0.5, 1, SECONDS, false, false};
  Protocol.InputRegisters[P3000_EAC_TODAY] = sGrowattModbusReg_t{
      3049,      0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EAC_TOTAL] = sGrowattModbusReg_t{
      3051,      0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EPV_TOTAL] = sGrowattModbusReg_t{
      3053,      0,     SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV1_TODAY] = sGrowattModbusReg_t{
      3055,      0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV1_TOTAL] = sGrowattModbusReg_t{
      3057,      0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV2_TODAY] = sGrowattModbusReg_t{
      3059,      0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV2_TOTAL] = sGrowattModbusReg_t{
      3061,      0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  // FRAGMENT 1: END

  // FRAGMENT 2: BEGIN
  Protocol.InputRegisters[P3000_EPV3_TODAY] = sGrowattModbusReg_t{
      3063,      0,     SIZE_32BIT, F("PV3EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV3_TOTAL] = sGrowattModbusReg_t{
      3065,      0,     SIZE_32BIT, F("PV3EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_ETOUSER_TODAY] = sGrowattModbusReg_t{
      3067,      0,    SIZE_32BIT, F("TodayEnergyToUser"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ETOUSER_TOTAL] = sGrowattModbusReg_t{
      3069,      0,    SIZE_32BIT, F("TotalEnergyToUser"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ETOGRID_TODAY] = sGrowattModbusReg_t{
      3071,      0,    SIZE_32BIT, F("TodayEnergyToGrid"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ETOGRID_TOTAL] = sGrowattModbusReg_t{
      3073,      0,    SIZE_32BIT, F("TotalEnergyToGrid"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ELOAD_TODAY] = sGrowattModbusReg_t{
      3075,      0,    SIZE_32BIT, F("TodayEnergyOfUserLoad"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ELOAD_TOTAL] = sGrowattModbusReg_t{
      3077,      0,    SIZE_32BIT, F("TotalEnergyOfUserLoad"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EPV4_TODAY] = sGrowattModbusReg_t{
      3079,      0,     SIZE_32BIT, F("PV4EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV4_TOTAL] = sGrowattModbusReg_t{
      3081,      0,     SIZE_32BIT, F("PV4EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_EPV_TODAY] = sGrowattModbusReg_t{
      3083,      0,     SIZE_32BIT, F("PVEnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_DERATING_MODE] =
      sGrowattModbusReg_t{3086, 0,    SIZE_16BIT, F("DeratingMode"), 1, 1,
                          NONE, true, false};  // TODO: decode
  Protocol.InputRegisters[P3000_ISO] = sGrowattModbusReg_t{
      3087, 0, SIZE_16BIT, F("PVISOValue"), 1, 1, RESISTANCE_K, false, false};
  Protocol.InputRegisters[P3000_DCI_R] = sGrowattModbusReg_t{
      3088, 0, SIZE_16BIT, F("RDCICurr"), 0.1, 0.1, CURRENT_M, false, false};
  Protocol.InputRegisters[P3000_DCI_S] = sGrowattModbusReg_t{
      3089, 0, SIZE_16BIT, F("SDCICurr"), 0.1, 0.1, CURRENT_M, false, false};
  Protocol.InputRegisters[P3000_DCI_T] = sGrowattModbusReg_t{
      3090, 0, SIZE_16BIT, F("TDCICurr"), 0.1, 0.1, CURRENT_M, false, false};
  Protocol.InputRegisters[P3000_GFCI] = sGrowattModbusReg_t{
      3091, 0, SIZE_16BIT, F("GFCICurr"), 1, 1, CURRENT_M, false, false};
  Protocol.InputRegisters[P3000_BUS_VOLTAGE] = sGrowattModbusReg_t{
      3092,    0,     SIZE_16BIT, F("TotalBusVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_TEMP1] = sGrowattModbusReg_t{
      3093,        0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
      TEMPERATURE, true, false};
  Protocol.InputRegisters[P3000_TEMP2] = sGrowattModbusReg_t{
      3094,        0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
      TEMPERATURE, false, false};
  Protocol.InputRegisters[P3000_TEMP3] = sGrowattModbusReg_t{
      3095,        0,     SIZE_16BIT, F("BoostTemperature"), 0.1, 0.1,
      TEMPERATURE, false, false};
  Protocol.InputRegisters[P3000_TEMP5] = sGrowattModbusReg_t{
      3097, 0,   SIZE_16BIT,  F("CommunicationBoardTemperature"),
      0.1,  0.1, TEMPERATURE, false,
      false};
  Protocol.InputRegisters[P3000_P_BUS_VOLTAGE] = sGrowattModbusReg_t{
      3098,    0,     SIZE_16BIT, F("PBusInsideVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_N_BUS_VOLTAGE] = sGrowattModbusReg_t{
      3099,    0,     SIZE_16BIT, F("NBusInsideVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P3000_IPF] = sGrowattModbusReg_t{
      3100, 0, SIZE_16BIT, F("InverterOutputPFNow"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P3000_REALOPPERCENT] = sGrowattModbusReg_t{
      3101,       0,    SIZE_16BIT_S, F("RealOutputPercent"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.InputRegisters[P3000_OPFULLWATT] = sGrowattModbusReg_t{
      3102,    0,    SIZE_32BIT, F("OutputMaxpowerLimited"), 0.1, 0.1,
      POWER_W, true, true};
  Protocol.InputRegisters[P3000_FAULT_MAINCODE] = sGrowattModbusReg_t{
      3105, 0, SIZE_16BIT, F("InverterFaultMaincode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_WARN_MAINCODE] = sGrowattModbusReg_t{
      3106, 0, SIZE_16BIT, F("InverterWarnMaincode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_AFCI_STATUS] = sGrowattModbusReg_t{
      3112, 0, SIZE_16BIT, F("AFCIStatus"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P3000_INV_START_DELAY] = sGrowattModbusReg_t{
      3115, 0, SIZE_16BIT, F("InvStartDelayTime"), 1, 1, SECONDS, false, false};
  Protocol.InputRegisters[P3000_BDC_ONOFFSTATE] = sGrowattModbusReg_t{
      3118, 0, SIZE_16BIT, F("BDCConnectState"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_DRYCONTACTSTATE] = sGrowattModbusReg_t{
      3119, 0, SIZE_16BIT, F("DryContactState"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P3000_PSELF] = sGrowattModbusReg_t{
      3121, 0, SIZE_32BIT, F("SelfUsePower"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P3000_ESYS_TODAY] = sGrowattModbusReg_t{
      3123,      0,     SIZE_32BIT, F("SystemEnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  // FRAGMENT 2: END

  // FRAGMENT 3: BEGIN
  Protocol.InputRegisters[P3000_EDISCHR_TODAY] = sGrowattModbusReg_t{
      3125,      0,    SIZE_32BIT, F("DischargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EDISCHR_TOTAL] = sGrowattModbusReg_t{
      3127,      0,    SIZE_32BIT, F("DischargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ECHR_TODAY] = sGrowattModbusReg_t{
      3129,      0,    SIZE_32BIT, F("ChargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ECHR_TOTAL] = sGrowattModbusReg_t{
      3131,      0,    SIZE_32BIT, F("ChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EACCHR_TODAY] = sGrowattModbusReg_t{
      3133,      0,    SIZE_32BIT, F("ACChargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_EACCHR_TOTAL] = sGrowattModbusReg_t{
      3135,      0,    SIZE_32BIT, F("ACChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P3000_ESYS_TOTAL] = sGrowattModbusReg_t{
      3137,      0,     SIZE_32BIT, F("SystemEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_ESELF_TODAY] = sGrowattModbusReg_t{
      3139,      0,     SIZE_32BIT, F("SelfOutputEnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_ESELF_TOTAL] = sGrowattModbusReg_t{
      3141,      0,     SIZE_32BIT, F("SelfOutputEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_PRIORITY] = sGrowattModbusReg_t{
      3144, 0, SIZE_16BIT, F("Priority"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_DERATINGMODE] = sGrowattModbusReg_t{
      3165, 0, SIZE_16BIT, F("BDCDeratingMode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_SYSSTATE] = sGrowattModbusReg_t{
      3166, 0, SIZE_16BIT, F("BDCSysState"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_SYSMODE] = sGrowattModbusReg_t{
      3166, 0, SIZE_16BIT, F("BDCSysMode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_FAULTCODE] = sGrowattModbusReg_t{
      3167, 0, SIZE_16BIT, F("BDCFaultCode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_WARNCODE] = sGrowattModbusReg_t{
      3168, 0, SIZE_16BIT, F("BDCWarnCode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P3000_BDC_VBAT] = sGrowattModbusReg_t{
      3169,    0,    SIZE_16BIT, F("BDCBatteryVoltage"), 0.1, 0.1,
      VOLTAGE, true, false};
  Protocol.InputRegisters[P3000_BDC_IBAT] = sGrowattModbusReg_t{
      3170,    0,    SIZE_16BIT_S, F("BDCBatteryCurrent"), 0.1, 0.1,
      CURRENT, true, false};
  Protocol.InputRegisters[P3000_BDC_SOC] = sGrowattModbusReg_t{
      3171,       0,    SIZE_16BIT, F("BDCStateOfCharge"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.InputRegisters[P3000_BDC_VBUS1] = sGrowattModbusReg_t{
      3172,    0,     SIZE_16BIT, F("BDCTotalBusVoltage"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_BDC_VBUS2] = sGrowattModbusReg_t{
      3173,    0,     SIZE_16BIT, F("BDCOnTheBusVoltage"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_BDC_IBB] = sGrowattModbusReg_t{
      3174,    0,     SIZE_16BIT, F("BDCBuckBoostCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P3000_BDC_ILLC] = sGrowattModbusReg_t{
      3175, 0, SIZE_16BIT, F("BDCLlcCurrent"), 0.1, 0.1, CURRENT, false, false};
  Protocol.InputRegisters[P3000_BDC_TEMPA] = sGrowattModbusReg_t{
      3176,        0,    SIZE_16BIT, F("BDCTemperatureA"), 0.1, 0.1,
      TEMPERATURE, true, false};
  Protocol.InputRegisters[P3000_BDC_TEMPB] = sGrowattModbusReg_t{
      3177,        0,     SIZE_16BIT, F("BDCTemperatureB"), 0.1, 0.1,
      TEMPERATURE, false, false};
  Protocol.InputRegisters[P3000_BDC_PDISCHR] = sGrowattModbusReg_t{
      3178,    0,    SIZE_32BIT, F("BDCDischargePower"), 0.1, 0.1,
      POWER_W, true, true};
  Protocol.InputRegisters[P3000_BDC_PCHR] = sGrowattModbusReg_t{
      3180, 0, SIZE_32BIT, F("BDCChargePower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P3000_BDC_EDISCHR_TOTAL] = sGrowattModbusReg_t{
      3182,      0,     SIZE_32BIT, F("BDCDischargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P3000_BDC_ECHR_TOTAL] = sGrowattModbusReg_t{
      3184,      0,     SIZE_32BIT, F("BDCChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  // FRAGMENT 3: END

  Protocol.InputReadFragments[Protocol.InputFragmentCount++] =
      sGrowattReadFragment_t{3000, 63};
  Protocol.InputReadFragments[Protocol.InputFragmentCount++] =
      sGrowattReadFragment_t{3063, 62};
  Protocol.InputReadFragments[Protocol.InputFragmentCount++] =
      sGrowattReadFragment_t{3125, 61};

  Protocol.HoldingRegisterCount = P3000_HOLING_REGISTER_COUNT;

  // FRAGMENT 1: BEGIN
  Protocol.HoldingRegisters[P3000_ACTIVE_P_RATE] = sGrowattModbusReg_t{
      3, 0, SIZE_16BIT, F("ActivePowerRate"), 1, 1, PERCENTAGE, true, false};
  // FRAGMENT 1: END

  // FRAGMENT 2: BEGIN
  // The BDCDischargePowerRate seems to apply to grid first AND load first mode.
  // The BDCDischargeStopSOC applies to grid first mode only. If you want to
  // prevent discharging set BDCDischargePowerRate to 0. The BDCChargePowerRate
  // seems to apply to battery first AND load first mode. The BDCChargeStopSOC
  // applies to battery first mode AND also to load first mode. If
  // BDCChargeACEnabled is enabled the battery is charged up to BDCChargeStopSOC
  // via AC.
  Protocol.HoldingRegisters[P3000_BDC_DISCHARGE_P_RATE] = sGrowattModbusReg_t{
      3036,       0,    SIZE_16BIT, F("BDCDischargePowerRate"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.HoldingRegisters[P3000_BDC_DISCHARGE_STOPSOC] = sGrowattModbusReg_t{
      3037,       0,    SIZE_16BIT, F("BDCDischargeStopSOC"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.HoldingRegisters[P3000_BDC_CHARGE_P_RATE] = sGrowattModbusReg_t{
      3047,       0,    SIZE_16BIT, F("BDCChargePowerRate"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.HoldingRegisters[P3000_BDC_CHARGE_STOPSOC] = sGrowattModbusReg_t{
      3048,       0,    SIZE_16BIT, F("BDCChargeStopSOC"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.HoldingRegisters[P3000_BDC_CHARGE_AC_ENABLED] = sGrowattModbusReg_t{
      3049, 0, SIZE_16BIT, F("BDCChargeACEnabled"), 1, 1, NONE, true, false};
  // FRAGMENT 2: END

  Protocol.HoldingRegisterCount = P3000_HOLING_REGISTER_COUNT;

  Protocol.HoldingReadFragments[Protocol.HoldingFragmentCount++] =
      sGrowattReadFragment_t{3, 1};
  Protocol.HoldingReadFragments[Protocol.HoldingFragmentCount++] =
      sGrowattReadFragment_t{3036, 14};

  // COMMANDS

  inverter.RegisterCommand("datetime/get", getDateTime);
  inverter.RegisterCommand("datetime/set", updateDateTime);

  inverter.RegisterCommand("power/get/activerate", getPowerActiveRate);
  inverter.RegisterCommand("power/set/activerate", setPowerActiveRate);

  inverter.RegisterCommand("bdc/set/dischargestopsoc", setBDCDischargeStopSOC);
  inverter.RegisterCommand("bdc/set/dischargepowerrate",
                           setBDCDischargePowerRate);

  inverter.RegisterCommand("bdc/set/chargestopsoc", setBDCChargeStopSOC);
  inverter.RegisterCommand("bdc/set/chargepowerrate", setBDCChargePowerRate);

  inverter.RegisterCommand("bdc/set/acchargeenabled", setBDCACChargeEnabled);

  inverter.RegisterCommand("priority/set", setPriority);

  Log.print(F("init_growattTLXH: number of input registers "));
  Log.print(Protocol.InputRegisterCount);
  Log.print(F(" number of holding registers "));
  Log.println(Protocol.HoldingRegisterCount);
}
