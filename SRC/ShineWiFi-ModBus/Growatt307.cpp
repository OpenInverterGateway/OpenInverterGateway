#include "Arduino.h"

#include "Config.h"
#include "Growatt.h"
#include "Growatt307.h"
#include <TLog.h>

// Helper function to verify register writes with retry
// Some Growatt firmwares commit values after a short delay
static bool verifyRegisters307(Growatt& inverter, uint16_t addr, uint16_t count,
                               const uint16_t* expected) {
  const int MAX_RETRIES = 3;
  const int RETRY_DELAY_MS = 50;

  for (int i = 0; i < MAX_RETRIES; ++i) {
    uint16_t readback[10] = {0};   // Sufficient for our use cases
    if (count > 10) return false;  // Safety check

    if (inverter.ReadHoldingRegFrag(addr, count, readback)) {
      bool match = true;
      for (uint16_t j = 0; j < count; ++j) {
        if (readback[j] != expected[j]) {
          match = false;
          break;
        }
      }
      if (match) return true;
    }

    if (i < MAX_RETRIES - 1) {
      delay(RETRY_DELAY_MS);
    }
  }
  return false;
}

// Helper function to format time register value to "HH:MM" string
String formatTimeSlot307(uint16_t timeReg) {
  int hours = (timeReg >> 8) & 0xFF;
  int minutes = timeReg & 0xFF;
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
  return String(buf);
}

// Helper function to format complete time slot info
String formatTimeSlotInfo307(uint16_t start, uint16_t stop, uint16_t enabled) {
  String result = formatTimeSlot307(start) + "-" + formatTimeSlot307(stop);
  result += enabled ? " (ON)" : " (OFF)";
  return result;
}

// Helper function to format date/time registers to ISO string
String formatDateTime307(uint16_t year, uint16_t month, uint16_t day,
                         uint16_t hour, uint16_t minute, uint16_t second) {
  char buf[64];
  // Year in register is 2-digit (e.g., 24 for 2024)
  uint16_t fullYear = year < 100 ? 2000 + year : year;
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u", fullYear, month,
           day, hour, minute, second);
  return String(buf);
}

std::tuple<bool, String> getDateTime307(const JsonDocument& req,
                                        JsonDocument& res, Growatt& inverter) {
  uint16_t year, month, day, hour, minute, second;

#if SIMULATE_INVERTER != 1
  uint16_t result[6];
  bool success = inverter.ReadHoldingRegFrag(45, 6, result);
  if (success) {
    year = result[0];
    month = result[1];
    day = result[2];
    hour = result[3];
    minute = result[4];
    second = result[5];
  }
#else
  year = 24;  // 2024
  month = 1;
  day = 20;
  hour = 14;
  minute = 30;
  second = 45;
  bool success = true;
#endif

  if (success) {
    res["value"] = formatDateTime307(year, month, day, hour, minute, second);
    return std::make_tuple(true, "Successfully read date/time");
  } else {
    return std::make_tuple(false, "Failed to read date/time");
  }
};

std::tuple<bool, String> updateDateTime307(const JsonDocument& req,
                                           JsonDocument& res,
                                           Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  String datetime = req["value"].as<String>();
  if (datetime.length() != 19) {
    return std::make_tuple(false, "Invalid datetime format");
  }

#if SIMULATE_INVERTER != 1
  uint16_t year = datetime.substring(0, 4).toInt();
  uint16_t month = datetime.substring(5, 7).toInt();
  uint16_t day = datetime.substring(8, 10).toInt();
  uint16_t hour = datetime.substring(11, 13).toInt();
  uint16_t minute = datetime.substring(14, 16).toInt();
  uint16_t second = datetime.substring(17, 19).toInt();

  year = year > 2000 ? year - 2000 : 0;

  uint16_t values[] = {year, month, day, hour, minute, second};

  bool success = inverter.WriteHoldingRegFrag(45, 6, values);
#else
  bool success = true;
#endif
  if (success) {
    return std::make_tuple(true, "Successfully updated date/time");
  } else {
    return std::make_tuple(false, "Failed to write date/time");
  }
};

std::tuple<bool, String> getPowerActiveRate307(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter) {
  uint16_t value;

#if SIMULATE_INVERTER != 1
  if (!inverter.ReadHoldingReg(3, &value)) {
    return std::make_tuple(false, "Failed to read active rate");
  }
#endif

  res["value"] = value;

  return std::make_tuple(true, "Successfully read active rate");
};

std::tuple<bool, String> setPowerActiveRate307(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  uint16_t value = req["value"].as<uint16_t>();

  // Valid range: 0-100% or 255 (follow schedule/unlimited)
  if (!((value >= 0 && value <= 100) || value == 255)) {
    return std::make_tuple(false, "'value' field not in range (0-100 or 255)");
  }

#if SIMULATE_INVERTER != 1
  if (!inverter.WriteHoldingReg(3, value)) {
    return std::make_tuple(false, "Failed to write active rate");
  }
#endif

  return std::make_tuple(true, "Successfully updated active rate");
};

std::tuple<bool, String> setExportEnable307(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  // First check if HR122 is set to 1, if not set it
  uint16_t currentFlag;
  if (inverter.ReadHoldingReg(122, &currentFlag)) {
    if (currentFlag != 1) {
      if (!inverter.WriteHoldingReg(122, 1)) {
        return std::make_tuple(false, "Failed to set HR122 flag");
      }
    }
  }

  // Set HR123 to 1000 (100% export allowed)
  if (!inverter.WriteHoldingReg(123, 1000)) {
    return std::make_tuple(false, "Failed to enable export");
  }
#endif
  return std::make_tuple(true, "Successfully enabled export");
}

std::tuple<bool, String> setExportDisable307(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  // First check if HR122 is set to 1, if not set it
  uint16_t currentFlag;
  if (inverter.ReadHoldingReg(122, &currentFlag)) {
    if (currentFlag != 1) {
      if (!inverter.WriteHoldingReg(122, 1)) {
        return std::make_tuple(false, "Failed to set HR122 flag");
      }
    }
  }

  // Set HR123 to 0 (0% export allowed)
  if (!inverter.WriteHoldingReg(123, 0)) {
    return std::make_tuple(false, "Failed to disable export");
  }
#endif
  return std::make_tuple(true, "Successfully disabled export");
}

std::tuple<String, String> getTimeSlot307(uint16_t start, uint16_t stop) {
  int start_hours = (start >> 8) & 0xFF;
  int start_minutes = start & 0xFF;
  int stop_hours = (stop >> 8) & 0xFF;
  int stop_minutes = stop & 0xFF;
  char start_str[8];
  char stop_str[8];
  snprintf(start_str, sizeof(start_str), "%02d:%02d", start_hours,
           start_minutes);
  snprintf(stop_str, sizeof(stop_str), "%02d:%02d", stop_hours, stop_minutes);
  return std::make_tuple(String(start_str), String(stop_str));
}

std::tuple<bool, String> getBatteryFirst307(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  uint16_t settings[3];
  if (!inverter.ReadHoldingRegFrag(1090, 3, settings)) {
    return std::make_tuple(false, "Failed to read battery first settings");
  }

  uint16_t powerRate = settings[0];
  uint16_t stopSOC = settings[1];
  bool acChargeEnabled = settings[2] == 1;
#else
  uint16_t powerRate = 100;
  uint16_t stopSOC = 75;
  bool acChargeEnabled = true;
#endif

  res["powerRate"] = powerRate;
  res["stopSOC"] = stopSOC;
  res["acChargeEnabled"] = acChargeEnabled;

#if SIMULATE_INVERTER != 1
  uint16_t timeslots_raw[9];
  if (!inverter.ReadHoldingRegFrag(1100, 9, timeslots_raw)) {
    return std::make_tuple(false, "Failed to read battery first timeslots");
  }
#endif

  auto timeslots = res.createNestedArray("timeSlots");
  for (int i = 0; i < 3; i++) {
    auto timeslot = timeslots.createNestedObject();
    String start_str;
    String stop_str;
    bool enabled;
#if SIMULATE_INVERTER != 1
    std::tie(start_str, stop_str) =
        getTimeSlot307(timeslots_raw[i * 3], timeslots_raw[i * 3 + 1]);
    enabled = timeslots_raw[i * 3 + 2] == 1;
#else
    start_str = "01:30";
    stop_str = "05:30";
    enabled = true;
#endif
    timeslot["slot"] = i + 1;
    timeslot["start"] = start_str;
    timeslot["stop"] = stop_str;
    timeslot["enabled"] = enabled;
  }

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstPowerRate307(const JsonDocument& req,
                                                     JsonDocument& res,
                                                     Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1090, value)) {
    return std::make_tuple(false, "Failed to write battery first power rate");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstStopSOC307(const JsonDocument& req,
                                                   JsonDocument& res,
                                                   Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1091, value)) {
    return std::make_tuple(false, "Failed to write battery first stop SOC");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstACChargeEnabled307(
    const JsonDocument& req, JsonDocument& res, Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1092, value)) {
    return std::make_tuple(false,
                           "Failed to write battery first AC charge enabled");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setTimeSlot307(const JsonDocument& req,
                                        JsonDocument& res, Growatt& inverter,
                                        uint16_t startReg) {
  if (!req.containsKey("start")) {
    return std::make_tuple(false, "'start' field is required");
  }
  String start_str = req["start"].as<String>();

  if (!req.containsKey("stop")) {
    return std::make_tuple(false, "'stop' field is required");
  }
  String stop_str = req["stop"].as<String>();

  if (!req.containsKey("enabled")) {
    return std::make_tuple(false, "'enabled' field is required");
  }

  if (start_str.length() != 5 || stop_str.length() != 5 ||
      start_str[2] != ':' || stop_str[2] != ':') {
    return std::make_tuple(false, "Invalid time format (must be HH:MM)");
  }

  if (!req.containsKey("slot")) {
    return std::make_tuple(false, "'slot' field is required");
  }
  uint16_t slot = req["slot"].as<uint16_t>();

  if (slot < 1 || slot > 3) {
    return std::make_tuple(false,
                           "Invalid slot number, must be between 1 and 3");
  }

#if SIMULATE_INVERTER != 1
  bool enabled = req["enabled"].as<bool>();

  int start_hours = start_str.substring(0, 2).toInt();
  int start_minutes = start_str.substring(3, 5).toInt();
  int stop_hours = stop_str.substring(0, 2).toInt();
  int stop_minutes = stop_str.substring(3, 5).toInt();

  // Validate time bounds
  if (start_hours < 0 || start_hours > 23 || start_minutes < 0 ||
      start_minutes > 59 || stop_hours < 0 || stop_hours > 23 ||
      stop_minutes < 0 || stop_minutes > 59) {
    return std::make_tuple(false,
                           "Invalid time values (hours: 0-23, minutes: 0-59)");
  }

  uint16_t time_start = (start_hours << 8) | start_minutes;
  uint16_t time_stop = (stop_hours << 8) | stop_minutes;

  uint16_t timeslot_raw[3];
  timeslot_raw[0] = time_start;
  timeslot_raw[1] = time_stop;
  timeslot_raw[2] = enabled ? 1 : 0;

  uint16_t timeslot_start_addr = startReg + ((slot - 1) * 3);
  if (!inverter.WriteHoldingRegFrag(timeslot_start_addr, 3, timeslot_raw)) {
    return std::make_tuple(false, "Failed to write timeslot");
  }

  // Verify the write succeeded
  if (!verifyRegisters307(inverter, timeslot_start_addr, 3, timeslot_raw)) {
    return std::make_tuple(false, "Timeslot verify failed");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstTimeSlot307(const JsonDocument& req,
                                                    JsonDocument& res,
                                                    Growatt& inverter) {
  return setTimeSlot307(req, res, inverter, 1100);
}

std::tuple<bool, String> getGridFirst307(const JsonDocument& req,
                                         JsonDocument& res, Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  uint16_t settings[2];  // Only reading 2 registers
  if (!inverter.ReadHoldingRegFrag(1070, 2, settings)) {
    return std::make_tuple(false, "Failed to read grid first settings");
  }

  uint16_t powerRate = settings[0];
  uint16_t stopSOC = settings[1];
#else
  uint16_t powerRate = 100;
  uint16_t stopSOC = 5;
#endif

  res["powerRate"] = powerRate;
  res["stopSOC"] = stopSOC;

#if SIMULATE_INVERTER != 1
  uint16_t timeslots_raw[9];
  if (!inverter.ReadHoldingRegFrag(1080, 9, timeslots_raw)) {
    return std::make_tuple(false, "Failed to read grid first timeslots");
  }
#endif

  auto timeslots = res.createNestedArray("timeSlots");
  for (int i = 0; i < 3; i++) {
    auto timeslot = timeslots.createNestedObject();
    String start_str;
    String stop_str;
    bool enabled;
#if SIMULATE_INVERTER != 1
    std::tie(start_str, stop_str) =
        getTimeSlot307(timeslots_raw[i * 3], timeslots_raw[i * 3 + 1]);
    enabled = timeslots_raw[i * 3 + 2] == 1;
#else
    start_str = "01:30";
    stop_str = "05:30";
    enabled = true;
#endif
    timeslot["slot"] = i + 1;
    timeslot["start"] = start_str;
    timeslot["stop"] = stop_str;
    timeslot["enabled"] = enabled;
  }

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstPowerRate307(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1070, value)) {
    return std::make_tuple(false, "Failed to write grid first power rate");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstStopSOC307(const JsonDocument& req,
                                                JsonDocument& res,
                                                Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1071, value)) {
    return std::make_tuple(false, "Failed to write grid first stop SOC");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstTimeSlot307(const JsonDocument& req,
                                                 JsonDocument& res,
                                                 Growatt& inverter) {
  return setTimeSlot307(req, res, inverter, 1080);
}

#ifndef TEMPERATURE_WORKAROUND_MULTIPLIER
#define TEMPERATURE_WORKAROUND_MULTIPLIER 0.1
#endif

void init_growatt307(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  // definition of input registers
  Protocol.InputRegisterCount = P307_INPUT_REGISTER_COUNT;
  // address, value, size, name, multiplier, unit, frontend, plot
  // FRAGMENT 1: BEGIN
  Protocol.InputRegisters[P307_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P307_INPUT_POWER] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("InputPower"), 0.1, 0.1, POWER_W, true, true};

  Protocol.InputRegisters[P307_PV1_VOLTAGE] = sGrowattModbusReg_t{
      3, 0, SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P307_PV1_CURRENT] = sGrowattModbusReg_t{
      4, 0, SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1, CURRENT, false, false};
  Protocol.InputRegisters[P307_PV1_POWER] = sGrowattModbusReg_t{
      5, 0, SIZE_32BIT, F("PV1InputPower"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P307_PV2_VOLTAGE] = sGrowattModbusReg_t{
      7, 0, SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P307_PV2_CURRENT] = sGrowattModbusReg_t{
      8, 0, SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1, CURRENT, false, false};
  Protocol.InputRegisters[P307_PV2_POWER] = sGrowattModbusReg_t{
      9, 0, SIZE_32BIT, F("PV2InputPower"), 0.1, 0.1, POWER_W, false, false};

  Protocol.InputRegisters[P307_PAC] = sGrowattModbusReg_t{
      35, 0, SIZE_32BIT_S, F("OutputPower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P307_FAC] = sGrowattModbusReg_t{
      37,        0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
      FREQUENCY, false, false};

  Protocol.InputRegisters[P307_VAC1] = sGrowattModbusReg_t{
      38,      0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P307_IAC1] = sGrowattModbusReg_t{
      39,      0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P307_PAC1] = sGrowattModbusReg_t{
      40, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};
  Protocol.InputRegisters[P307_VAC2] = sGrowattModbusReg_t{
      42,      0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P307_IAC2] = sGrowattModbusReg_t{
      43,      0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P307_PAC2] = sGrowattModbusReg_t{
      44, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};
  Protocol.InputRegisters[P307_VAC3] = sGrowattModbusReg_t{
      46,      0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P307_IAC3] = sGrowattModbusReg_t{
      47,      0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
      CURRENT, false, false};
  Protocol.InputRegisters[P307_PAC3] = sGrowattModbusReg_t{
      48, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
      VA, false, false};
  // FRAGMENT 1: END

  // FRAGMENT 2: BEGIN
  Protocol.InputRegisters[P307_EAC_TODAY] = sGrowattModbusReg_t{
      53,        0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_EAC_TOTAL] = sGrowattModbusReg_t{
      55,        0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_TIME_TOTAL] = sGrowattModbusReg_t{
      57, 0, SIZE_32BIT, F("TWorkTimeTotal"), 0.5, 1, SECONDS, false, false};

  Protocol.InputRegisters[P307_EPV1_TODAY] = sGrowattModbusReg_t{
      59,        0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P307_EPV1_TOTAL] = sGrowattModbusReg_t{
      61,        0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P307_EPV2_TODAY] = sGrowattModbusReg_t{
      63,        0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P307_EPV2_TOTAL] = sGrowattModbusReg_t{
      65,        0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
      POWER_KWH, false, false};
  Protocol.InputRegisters[P307_EPV_TOTAL] = sGrowattModbusReg_t{
      91, 0, SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1, POWER_KWH, false, false};

  Protocol.InputRegisters[P307_TEMP1] = sGrowattModbusReg_t{
      93,          0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
      TEMPERATURE, true, true};
  Protocol.InputRegisters[P307_TEMP2] = sGrowattModbusReg_t{
      94,          0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
      TEMPERATURE, false, false};
  Protocol.InputRegisters[P307_TEMP3] = sGrowattModbusReg_t{
      95,          0,     SIZE_16BIT, F("BoostTemperature"), 0.1, 0.1,
      TEMPERATURE, false, false};
  // FRAGMENT 2: END

  // FRAGMENT 3: BEGIN
  Protocol.InputRegisters[P307_PDISCHARGE] = sGrowattModbusReg_t{
      1009, 0, SIZE_32BIT, F("DischargePower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P307_PCHARGE] = sGrowattModbusReg_t{
      1011, 0, SIZE_32BIT, F("ChargePower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P307_VBAT] = sGrowattModbusReg_t{
      1013,    0,     SIZE_16BIT, F("BatteryVoltage"), 0.1, 0.1,
      VOLTAGE, false, false};
  Protocol.InputRegisters[P307_SOC] = sGrowattModbusReg_t{
      1014, 0, SIZE_16BIT, F("SOC"), 1, 1, PERCENTAGE, true, true};
  Protocol.InputRegisters[P307_PAC_TO_USER] = sGrowattModbusReg_t{
      1015, 0, SIZE_32BIT, F("ACPowerToUser"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P307_PAC_TO_USER_TOTAL] = sGrowattModbusReg_t{
      1021,    0,     SIZE_32BIT, F("ACPowerToUserTotal"), 0.1, 0.1,
      POWER_W, false, false};
  Protocol.InputRegisters[P307_PAC_TO_GRID] = sGrowattModbusReg_t{
      1023, 0, SIZE_32BIT, F("ACPowerToGrid"), 0.1, 0.1, POWER_W, false, false};
  Protocol.InputRegisters[P307_PAC_TO_GRID_TOTAL] = sGrowattModbusReg_t{
      1029,    0,     SIZE_32BIT, F("ACPowerToGridTotal"), 0.1, 0.1,
      POWER_W, false, false};
  Protocol.InputRegisters[P307_PLOCAL_LOAD] = sGrowattModbusReg_t{
      1031,    0,     SIZE_32BIT, F("INVPowerToLocalLoad"), 0.1, 0.1,
      POWER_W, false, false};
  Protocol.InputRegisters[P307_PLOCAL_LOAD_TOTAL] = sGrowattModbusReg_t{
      1037,    0,    SIZE_32BIT, F("INVPowerToLocalLoadTotal"), 0.1, 0.1,
      POWER_W, true, false};
  Protocol.InputRegisters[P307_BATTERY_TEMPERATURE] =
      sGrowattModbusReg_t{1040,
                          0,
                          SIZE_16BIT,
                          F("BatteryTemperature"),
                          TEMPERATURE_WORKAROUND_MULTIPLIER,
                          TEMPERATURE_WORKAROUND_MULTIPLIER,
                          TEMPERATURE,
                          true,
                          true};
  Protocol.InputRegisters[P307_BATTERY_STATE] = sGrowattModbusReg_t{
      1041, 0, SIZE_16BIT, F("BatteryState"), 1, 1, NONE, true, false};

  Protocol.InputRegisters[P307_ETOUSER_TODAY] = sGrowattModbusReg_t{
      1044,      0,    SIZE_32BIT, F("EnergyToUserToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ETOUSER_TOTAL] = sGrowattModbusReg_t{
      1046,      0,    SIZE_32BIT, F("EnergyToUserTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ETOGRID_TODAY] = sGrowattModbusReg_t{
      1048,      0,    SIZE_32BIT, F("EnergyToGridToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ETOGRID_TOTAL] = sGrowattModbusReg_t{
      1050,      0,    SIZE_32BIT, F("EnergyToGridTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_EDISCHARGE_TODAY] = sGrowattModbusReg_t{
      1052,      0,    SIZE_32BIT, F("DischargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_EDISCHARGE_TOTAL] = sGrowattModbusReg_t{
      1054,      0,    SIZE_32BIT, F("DischargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ECHARGE_TODAY] = sGrowattModbusReg_t{
      1056,      0,    SIZE_32BIT, F("ChargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ECHARGE_TOTAL] = sGrowattModbusReg_t{
      1058,      0,    SIZE_32BIT, F("ChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ETOLOCALLOAD_TODAY] = sGrowattModbusReg_t{
      1060,      0,    SIZE_32BIT, F("LocalLoadEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ETOLOCALLOAD_TOTAL] = sGrowattModbusReg_t{
      1062,      0,    SIZE_32BIT, F("LocalLoadEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  // FRAGMENT 3: END

  // FRAGMENT 4: START
  Protocol.InputRegisters[P307_ACCHARGE_TODAY] = sGrowattModbusReg_t{
      1124,      0,    SIZE_32BIT, F("ACChargeEnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};
  Protocol.InputRegisters[P307_ACCHARGE_TOTAL] = sGrowattModbusReg_t{
      1126,      0,    SIZE_32BIT, F("ACChargeEnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};
  // FRAGMENT 4: END

  // FRAGMENT 5: Current Mode Register
  Protocol.InputRegisters[P307_CURRENT_MODE] = sGrowattModbusReg_t{
      118, 0, SIZE_16BIT, F("CurrentMode"), 1, 1, NONE, true, false};
  // 0=Load-first, 1=Battery-first, 2=Grid-first

  Protocol.InputFragmentCount = 5;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 50};
  Protocol.InputReadFragments[1] = sGrowattReadFragment_t{53, 43};
  Protocol.InputReadFragments[2] =
      sGrowattReadFragment_t{118, 1};  // Current mode
  Protocol.InputReadFragments[3] = sGrowattReadFragment_t{1009, 55};
  Protocol.InputReadFragments[4] = sGrowattReadFragment_t{1124, 4};

  // definition of holding registers
  Protocol.HoldingRegisterCount = P307_HOLDING_REGISTER_COUNT;

  // FRAGMENT 1: Active Power Rate
  Protocol.HoldingRegisters[P307_Active_P_Rate] = sGrowattModbusReg_t{
      3, 0, SIZE_16BIT, F("ActivePowerRate"), 1, 1, PERCENTAGE, true, false};

  // FRAGMENT 2: System Date/Time
  Protocol.HoldingRegisters[P307_H_SYSTEM_YEAR] = sGrowattModbusReg_t{
      45, 0, SIZE_16BIT, F("SystemYear"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_SYSTEM_MONTH] = sGrowattModbusReg_t{
      46, 0, SIZE_16BIT, F("SystemMonth"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_SYSTEM_DAY] = sGrowattModbusReg_t{
      47, 0, SIZE_16BIT, F("SystemDay"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_SYSTEM_HOUR] = sGrowattModbusReg_t{
      48, 0, SIZE_16BIT, F("SystemHour"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_SYSTEM_MINUTE] = sGrowattModbusReg_t{
      49, 0, SIZE_16BIT, F("SystemMinute"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_SYSTEM_SECOND] = sGrowattModbusReg_t{
      50, 0, SIZE_16BIT, F("SystemSecond"), 1, 1, NONE, false, false};

  // FRAGMENT 3: Export Limit (122-123 for protocol 3.07)
  Protocol.HoldingRegisters[P307_H_EXPORT_LIMIT_ENABLED] = sGrowattModbusReg_t{
      122, 0, SIZE_16BIT, F("ExportLimitFlag"), 1, 1, NONE, true, false};
  Protocol.HoldingRegisters[P307_H_EXPORT_LIMIT_VALUE] = sGrowattModbusReg_t{
      123,        0,    SIZE_16BIT, F("ExportLimitValue"), 0.1, 0.1,
      PERCENTAGE, true, false};

  // FRAGMENT 4: Grid First settings
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_POWER_RATE] = sGrowattModbusReg_t{
      1070,       0,    SIZE_16BIT, F("GridFirstPwrRate"), 1, 1,
      PERCENTAGE, true, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_STOP_SOC] = sGrowattModbusReg_t{
      1071, 0, SIZE_16BIT, F("GridFirstSOC"), 1, 1, PERCENTAGE, true, false};

  // FRAGMENT 5: Grid First time slots
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT1_START] =
      sGrowattModbusReg_t{1080, 0,     SIZE_16BIT, F("GridSlot1Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT1_STOP] = sGrowattModbusReg_t{
      1081, 0, SIZE_16BIT, F("GridSlot1Stop"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT1_ENABLED] =
      sGrowattModbusReg_t{1082, 0,     SIZE_16BIT, F("GridSlot1En"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT2_START] =
      sGrowattModbusReg_t{1083, 0,     SIZE_16BIT, F("GridSlot2Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT2_STOP] = sGrowattModbusReg_t{
      1084, 0, SIZE_16BIT, F("GridSlot2Stop"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT2_ENABLED] =
      sGrowattModbusReg_t{1085, 0,     SIZE_16BIT, F("GridSlot2En"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT3_START] =
      sGrowattModbusReg_t{1086, 0,     SIZE_16BIT, F("GridSlot3Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT3_STOP] = sGrowattModbusReg_t{
      1087, 0, SIZE_16BIT, F("GridSlot3Stop"), 1, 1, NONE, false, false};
  Protocol.HoldingRegisters[P307_H_GRID_FIRST_SLOT3_ENABLED] =
      sGrowattModbusReg_t{1088, 0,     SIZE_16BIT, F("GridSlot3En"), 1, 1,
                          NONE, false, false};

  // FRAGMENT 6: Battery First settings
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_POWER_RATE] =
      sGrowattModbusReg_t{1090, 0, SIZE_16BIT, F("BattFirstPwrRate"),
                          1,    1, PERCENTAGE, false,
                          false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_STOP_SOC] =
      sGrowattModbusReg_t{1091, 0, SIZE_16BIT, F("BattFirstSOC"),
                          1,    1, PERCENTAGE, false,
                          false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_AC_CHARGE] =
      sGrowattModbusReg_t{1092, 0,     SIZE_16BIT, F("BattFirstACChrg"), 1, 1,
                          NONE, false, false};

  // FRAGMENT 7: Battery First time slots
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT1_START] =
      sGrowattModbusReg_t{1100, 0,     SIZE_16BIT, F("BattSlot1Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT1_STOP] =
      sGrowattModbusReg_t{1101, 0,     SIZE_16BIT, F("BattSlot1Stop"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT1_ENABLED] =
      sGrowattModbusReg_t{1102, 0,     SIZE_16BIT, F("BattSlot1En"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT2_START] =
      sGrowattModbusReg_t{1103, 0,     SIZE_16BIT, F("BattSlot2Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT2_STOP] =
      sGrowattModbusReg_t{1104, 0,     SIZE_16BIT, F("BattSlot2Stop"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT2_ENABLED] =
      sGrowattModbusReg_t{1105, 0,     SIZE_16BIT, F("BattSlot2En"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT3_START] =
      sGrowattModbusReg_t{1106, 0,     SIZE_16BIT, F("BattSlot3Start"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT3_STOP] =
      sGrowattModbusReg_t{1107, 0,     SIZE_16BIT, F("BattSlot3Stop"), 1, 1,
                          NONE, false, false};
  Protocol.HoldingRegisters[P307_H_BATTERY_FIRST_SLOT3_ENABLED] =
      sGrowattModbusReg_t{1108, 0,     SIZE_16BIT, F("BattSlot3En"), 1, 1,
                          NONE, false, false};

  Protocol.HoldingFragmentCount = 7;
  Protocol.HoldingReadFragments[0] =
      sGrowattReadFragment_t{3, 1};  // Active Power Rate
  Protocol.HoldingReadFragments[1] =
      sGrowattReadFragment_t{45, 6};  // Date/Time
  Protocol.HoldingReadFragments[2] =
      sGrowattReadFragment_t{122, 2};  // Export Limit (122-123)
  Protocol.HoldingReadFragments[3] =
      sGrowattReadFragment_t{1070, 2};  // Grid First settings
  Protocol.HoldingReadFragments[4] =
      sGrowattReadFragment_t{1080, 9};  // Grid First time slots
  Protocol.HoldingReadFragments[5] =
      sGrowattReadFragment_t{1090, 3};  // Battery First settings
  Protocol.HoldingReadFragments[6] =
      sGrowattReadFragment_t{1100, 9};  // Battery First time slots

  // definition of commands
  inverter.RegisterCommand("datetime/get", getDateTime307);
  inverter.RegisterCommand("datetime/set", updateDateTime307);

  inverter.RegisterCommand("batteryfirst/get", getBatteryFirst307);
  inverter.RegisterCommand("batteryfirst/set/powerrate",
                           setBatteryFirstPowerRate307);
  inverter.RegisterCommand("batteryfirst/set/stopsoc",
                           setBatteryFirstStopSOC307);
  inverter.RegisterCommand("batteryfirst/set/acchargeenabled",
                           setBatteryFirstACChargeEnabled307);
  inverter.RegisterCommand("batteryfirst/set/timeslot",
                           setBatteryFirstTimeSlot307);

  inverter.RegisterCommand("gridfirst/get", getGridFirst307);
  inverter.RegisterCommand("gridfirst/set/powerrate", setGridFirstPowerRate307);
  inverter.RegisterCommand("gridfirst/set/stopsoc", setGridFirstStopSOC307);
  inverter.RegisterCommand("gridfirst/set/timeslot", setGridFirstTimeSlot307);

  inverter.RegisterCommand("power/get/activerate", getPowerActiveRate307);
  inverter.RegisterCommand("power/set/activerate", setPowerActiveRate307);

  inverter.RegisterCommand("export/enable", setExportEnable307);
  inverter.RegisterCommand("export/disable", setExportDisable307);

  Log.print(F("init_growatt307: input registers "));
  Log.print(Protocol.InputRegisterCount);
  Log.print(F(" holding registers "));
  Log.println(Protocol.HoldingFragmentCount);
}