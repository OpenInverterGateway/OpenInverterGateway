#include "Arduino.h"

#include "Growatt.h"
#include "Growatt305.h"

void init_growatt305(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  // definition of input registers — set below after all entries are added
  // address, value, size, name, multiplier, unit, frontend, plot
  // FRAGMENT 1: BEGIN
  Protocol.InputRegisters[P305_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};  // #1
  Protocol.InputRegisters[P305_DC_POWER] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("DcPower"), 0.1, 0.1, POWER_W, true, true};  // #2
  Protocol.InputRegisters[P305_DC_VOLTAGE] = sGrowattModbusReg_t{
      3, 0, SIZE_16BIT, F("DcVoltage"), 0.1, 0.1, VOLTAGE, true, false};  // #3
  Protocol.InputRegisters[P305_DC_INPUT_CURRENT] = sGrowattModbusReg_t{
      4,       0,    SIZE_16BIT, F("DcInputCurrent"), 0.1, 0.1,
      CURRENT, true, false};  // #4
  Protocol.InputRegisters[P305_AC_FREQUENCY] = sGrowattModbusReg_t{
      13,        0,    SIZE_16BIT, F("AcFrequency"), 0.01, 0.01,
      FREQUENCY, true, false};  // #5
  Protocol.InputRegisters[P305_AC_VOLTAGE] = sGrowattModbusReg_t{
      14, 0, SIZE_16BIT, F("AcVoltage"), 0.1, 0.1, VOLTAGE, true, false};  // #6
  Protocol.InputRegisters[P305_AC_OUTPUT_CURRENT] = sGrowattModbusReg_t{
      15,      0,    SIZE_16BIT, F("AcOutputCurrent"), 0.1, 0.1,
      CURRENT, true, false};  // #7
  Protocol.InputRegisters[P305_AC_POWER] = sGrowattModbusReg_t{
      16, 0, SIZE_32BIT, F("AcPower"), 0.1, 0.1, POWER_W, true, true};  // #8
  Protocol.InputRegisters[P305_ENERGY_TODAY] = sGrowattModbusReg_t{
      26,        0,    SIZE_32BIT, F("EnergyToday"), 0.1, 0.1,
      POWER_KWH, true, false};  // #9
  Protocol.InputRegisters[P305_ENERGY_TOTAL] = sGrowattModbusReg_t{
      28,        0,    SIZE_32BIT, F("EnergyTotal"), 0.1, 0.1,
      POWER_KWH, true, false};  // #10
  Protocol.InputRegisters[P305_OPERATING_TIME] =
      sGrowattModbusReg_t{30,      0,    SIZE_32BIT, F("OperatingTime"), 0.5, 1,
                          SECONDS, true, false};  // #11
  Protocol.InputRegisters[P305_TEMPERATURE] = sGrowattModbusReg_t{
      32,          0,    SIZE_16BIT, F("Temperature"), 0.1, 0.1,
      TEMPERATURE, true, false};  // #12

  // Additional registers needed by the Growatt cloud DATA frame (T06NNNN format)
  // and to surface 3-phase data on the dashboard.
  // The cloud reads "pac" from registers 11-12 and per-phase data from 18-25;
  // without these entries the cloud sees zeros for total AC power on TL3 inverters
  // and the dashboard /uiStatus omits the L2/L3 columns entirely.
  Protocol.InputRegisters[P305_PAC_TOTAL] = sGrowattModbusReg_t{
      11, 0, SIZE_32BIT, F("PacTotal"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P305_PV_POWER_IN] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("PpvTotal"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_PV1_WATT] = sGrowattModbusReg_t{
      5, 0, SIZE_32BIT, F("Pv1Watt"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_VAC_L2] = sGrowattModbusReg_t{
      18, 0, SIZE_16BIT, F("AcVoltageL2"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_IAC_L2] = sGrowattModbusReg_t{
      19, 0, SIZE_16BIT, F("AcCurrentL2"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_PAC_L2] = sGrowattModbusReg_t{
      20, 0, SIZE_32BIT, F("AcPowerL2"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_VAC_L3] = sGrowattModbusReg_t{
      22, 0, SIZE_16BIT, F("AcVoltageL3"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_IAC_L3] = sGrowattModbusReg_t{
      23, 0, SIZE_16BIT, F("AcCurrentL3"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_PAC_L3] = sGrowattModbusReg_t{
      24, 0, SIZE_32BIT, F("AcPowerL3"), 0.1, 0.1, POWER_W, true, false};

  Protocol.InputRegisterCount = 21;
  Protocol.InputFragmentCount = 1;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 33};

  Protocol.HoldingRegisterCount = 0;
  Protocol.HoldingFragmentCount = 0;
}
