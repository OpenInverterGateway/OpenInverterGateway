#pragma once

#include "Arduino.h"

#include "Growatt.h"
#include "GrowattTypes.h"

// Growatt modbus protocol version 3.07 based on 1.24 with export limit changes
typedef enum {
  P307_I_STATUS = 0,
  P307_INPUT_POWER,
  P307_PV1_VOLTAGE,
  P307_PV1_CURRENT,
  P307_PV1_POWER,
  P307_PV2_VOLTAGE,
  P307_PV2_CURRENT,
  P307_PV2_POWER,
  P307_PAC,
  P307_FAC,
  P307_VAC1,
  P307_IAC1,
  P307_PAC1,
  P307_VAC2,
  P307_IAC2,
  P307_PAC2,
  P307_VAC3,
  P307_IAC3,
  P307_PAC3,
  P307_EAC_TODAY,
  P307_EAC_TOTAL,
  P307_TIME_TOTAL,
  P307_EPV1_TODAY,
  P307_EPV1_TOTAL,
  P307_EPV2_TODAY,
  P307_EPV2_TOTAL,
  P307_EPV_TOTAL,
  P307_TEMP1,
  P307_TEMP2,
  P307_TEMP3,
  P307_PDISCHARGE,
  P307_PCHARGE,
  P307_VBAT,
  P307_SOC,
  P307_PAC_TO_USER,
  P307_PAC_TO_USER_TOTAL,
  P307_PAC_TO_GRID,
  P307_PAC_TO_GRID_TOTAL,
  P307_PLOCAL_LOAD,
  P307_PLOCAL_LOAD_TOTAL,
  P307_BATTERY_TEMPERATURE,
  P307_BATTERY_STATE,
  P307_ETOUSER_TODAY,
  P307_ETOUSER_TOTAL,
  P307_ETOGRID_TODAY,
  P307_ETOGRID_TOTAL,
  P307_EDISCHARGE_TODAY,
  P307_EDISCHARGE_TOTAL,
  P307_ECHARGE_TODAY,
  P307_ECHARGE_TOTAL,
  P307_ETOLOCALLOAD_TODAY,
  P307_ETOLOCALLOAD_TOTAL,
  P307_ACCHARGE_TODAY,
  P307_ACCHARGE_TOTAL,
  P307_CURRENT_MODE,  // 0=Load-first, 1=Battery-first, 2=Grid-first
  P307_INPUT_REGISTER_COUNT
} eP307InputRegisters_t;

typedef enum {
  P307_Active_P_Rate,
  P307_H_SYSTEM_YEAR,
  P307_H_SYSTEM_MONTH,
  P307_H_SYSTEM_DAY,
  P307_H_SYSTEM_HOUR,
  P307_H_SYSTEM_MINUTE,
  P307_H_SYSTEM_SECOND,
  P307_H_EXPORT_LIMIT_VALUE,
  P307_H_EXPORT_LIMIT_ENABLED,
  P307_H_BATTERY_FIRST_POWER_RATE,
  P307_H_BATTERY_FIRST_STOP_SOC,
  P307_H_BATTERY_FIRST_AC_CHARGE,
  P307_H_BATTERY_FIRST_SLOT1_START,
  P307_H_BATTERY_FIRST_SLOT1_STOP,
  P307_H_BATTERY_FIRST_SLOT1_ENABLED,
  P307_H_BATTERY_FIRST_SLOT2_START,
  P307_H_BATTERY_FIRST_SLOT2_STOP,
  P307_H_BATTERY_FIRST_SLOT2_ENABLED,
  P307_H_BATTERY_FIRST_SLOT3_START,
  P307_H_BATTERY_FIRST_SLOT3_STOP,
  P307_H_BATTERY_FIRST_SLOT3_ENABLED,
  P307_H_GRID_FIRST_POWER_RATE,
  P307_H_GRID_FIRST_STOP_SOC,
  P307_H_GRID_FIRST_SLOT1_START,
  P307_H_GRID_FIRST_SLOT1_STOP,
  P307_H_GRID_FIRST_SLOT1_ENABLED,
  P307_H_GRID_FIRST_SLOT2_START,
  P307_H_GRID_FIRST_SLOT2_STOP,
  P307_H_GRID_FIRST_SLOT2_ENABLED,
  P307_H_GRID_FIRST_SLOT3_START,
  P307_H_GRID_FIRST_SLOT3_STOP,
  P307_H_GRID_FIRST_SLOT3_ENABLED,
  P307_HOLDING_REGISTER_COUNT
} eP307HoldingRegisters_t;

void init_growatt307(sProtocolDefinition_t& Protocol, Growatt& inverter);

std::tuple<bool, String> getDateTime307(const JsonDocument& req, JsonDocument& res,
                                     Growatt& inverter);
std::tuple<bool, String> updateDateTime307(const JsonDocument& req,
                                        JsonDocument& res, Growatt& inverter);
std::tuple<bool, String> getPowerActiveRate307(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter);
std::tuple<bool, String> setPowerActiveRate307(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter);
std::tuple<bool, String> setExportEnable307(const JsonDocument& req,
                                        JsonDocument& res,
                                        Growatt& inverter);
std::tuple<bool, String> setExportDisable307(const JsonDocument& req,
                                         JsonDocument& res,
                                         Growatt& inverter);
std::tuple<bool, String> setExportLimit307(const JsonDocument& req,
                                           JsonDocument& res,
                                           Growatt& inverter);
std::tuple<bool, String> getBatteryFirst307(const JsonDocument& req,
                                         JsonDocument& res, Growatt& inverter);
std::tuple<bool, String> setBatteryFirstPowerRate307(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter);
std::tuple<bool, String> setBatteryFirstStopSOC307(const JsonDocument& req,
                                                JsonDocument& res,
                                                Growatt& inverter);
std::tuple<bool, String> setBatteryFirstACChargeEnabled307(const JsonDocument& req,
                                                        JsonDocument& res,
                                                        Growatt& inverter);
std::tuple<bool, String> setBatteryFirstTimeSlot307(const JsonDocument& req,
                                                 JsonDocument& res,
                                                 Growatt& inverter);
std::tuple<bool, String> getGridFirst307(const JsonDocument& req,
                                      JsonDocument& res, Growatt& inverter);
std::tuple<bool, String> setGridFirstPowerRate307(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter);
std::tuple<bool, String> setGridFirstStopSOC307(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter);
std::tuple<bool, String> setGridFirstTimeSlot307(const JsonDocument& req,
                                              JsonDocument& res,
                                              Growatt& inverter);