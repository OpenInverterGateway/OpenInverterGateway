#include "Arduino.h"

#include "Growatt.h"
#include "Growatt305.h"

void init_growatt305(sProtocolDefinition_t& Protocol, Growatt& inverter) {
  // Input register map for Growatt v3.05 protocol, block 1 (regs 0-44).
  // Block 2 (47-89) is fetched separately by the .ino because this inverter's
  // FC04 response to start=45 returns data shifted by +2 (see ShineWiFi-ModBus
  // .ino cloud-feed section).
  //
  // address, value, size, name, multiplier, resolution, unit, frontend, plot
  // Entries MUST stay sorted by address (Growatt::ReadInputRegisters j-walk).

  Protocol.InputRegisterCount = 34;

  Protocol.InputRegisters[P305_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P305_DC_POWER] = sGrowattModbusReg_t{
      1, 0, SIZE_32BIT, F("DcPower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P305_DC_VOLTAGE] = sGrowattModbusReg_t{
      3, 0, SIZE_16BIT, F("DcVoltage"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_DC_INPUT_CURRENT] = sGrowattModbusReg_t{
      4, 0, SIZE_16BIT, F("DcInputCurrent"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_PV1_POWER] = sGrowattModbusReg_t{
      5, 0, SIZE_32BIT, F("Pv1Power"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_PV2_VOLTAGE] = sGrowattModbusReg_t{
      7, 0, SIZE_16BIT, F("Pv2Voltage"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_PV2_CURRENT] = sGrowattModbusReg_t{
      8, 0, SIZE_16BIT, F("Pv2Current"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_PV2_POWER] = sGrowattModbusReg_t{
      9, 0, SIZE_32BIT, F("Pv2Power"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_PAC_TOTAL] = sGrowattModbusReg_t{
      11, 0, SIZE_32BIT, F("PacTotal"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P305_AC_FREQUENCY] = sGrowattModbusReg_t{
      13, 0, SIZE_16BIT, F("AcFrequency"), 0.01, 0.01, FREQUENCY, true, false};
  Protocol.InputRegisters[P305_AC_VOLTAGE] = sGrowattModbusReg_t{
      14, 0, SIZE_16BIT, F("AcVoltage"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_AC_OUTPUT_CURRENT] = sGrowattModbusReg_t{
      15, 0, SIZE_16BIT, F("AcOutputCurrent"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_AC_POWER] = sGrowattModbusReg_t{
      16, 0, SIZE_32BIT, F("AcPower"), 0.1, 0.1, POWER_W, true, true};
  Protocol.InputRegisters[P305_AC_VOLTAGE_L2] = sGrowattModbusReg_t{
      18, 0, SIZE_16BIT, F("AcVoltageL2"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_AC_CURRENT_L2] = sGrowattModbusReg_t{
      19, 0, SIZE_16BIT, F("AcCurrentL2"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_AC_POWER_L2] = sGrowattModbusReg_t{
      20, 0, SIZE_32BIT, F("AcPowerL2"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_AC_VOLTAGE_L3] = sGrowattModbusReg_t{
      22, 0, SIZE_16BIT, F("AcVoltageL3"), 0.1, 0.1, VOLTAGE, true, false};
  Protocol.InputRegisters[P305_AC_CURRENT_L3] = sGrowattModbusReg_t{
      23, 0, SIZE_16BIT, F("AcCurrentL3"), 0.1, 0.1, CURRENT, true, false};
  Protocol.InputRegisters[P305_AC_POWER_L3] = sGrowattModbusReg_t{
      24, 0, SIZE_32BIT, F("AcPowerL3"), 0.1, 0.1, POWER_W, true, false};
  Protocol.InputRegisters[P305_ENERGY_TODAY] = sGrowattModbusReg_t{
      26, 0, SIZE_32BIT, F("EnergyToday"), 0.1, 0.1, POWER_KWH, true, false};
  Protocol.InputRegisters[P305_ENERGY_TOTAL] = sGrowattModbusReg_t{
      28, 0, SIZE_32BIT, F("EnergyTotal"), 0.1, 0.1, POWER_KWH, true, false};
  Protocol.InputRegisters[P305_OPERATING_TIME] = sGrowattModbusReg_t{
      30, 0, SIZE_32BIT, F("OperatingTime"), 0.5, 1, SECONDS, true, false};
  Protocol.InputRegisters[P305_TEMPERATURE] = sGrowattModbusReg_t{
      32, 0, SIZE_16BIT, F("Temperature"), 0.1, 0.1, TEMPERATURE, true, false};
  Protocol.InputRegisters[P305_ISO_FAULT] = sGrowattModbusReg_t{
      33, 0, SIZE_16BIT, F("IsoFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_GFCI_FAULT] = sGrowattModbusReg_t{
      34, 0, SIZE_16BIT, F("GfciFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_DCI_FAULT] = sGrowattModbusReg_t{
      35, 0, SIZE_16BIT, F("DciFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_VPV_FAULT] = sGrowattModbusReg_t{
      36, 0, SIZE_16BIT, F("VpvFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_VAC_FAULT] = sGrowattModbusReg_t{
      37, 0, SIZE_16BIT, F("VacFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_FAC_FAULT] = sGrowattModbusReg_t{
      38, 0, SIZE_16BIT, F("FacFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_TMP_FAULT] = sGrowattModbusReg_t{
      39, 0, SIZE_16BIT, F("TmpFault"), 1, 1, NONE, false, false};
  Protocol.InputRegisters[P305_FAULT_CODE] = sGrowattModbusReg_t{
      40, 0, SIZE_16BIT, F("FaultCode"), 1, 1, NONE, true, false};
  Protocol.InputRegisters[P305_IPM_TEMPERATURE] = sGrowattModbusReg_t{
      41,          0,    SIZE_16BIT, F("IpmTemperature"), 0.1, 0.1,
      TEMPERATURE, true, false};
  Protocol.InputRegisters[P305_PBUS_VOLTAGE] = sGrowattModbusReg_t{
      42, 0, SIZE_16BIT, F("PBusVoltage"), 0.1, 0.1, VOLTAGE, false, false};
  Protocol.InputRegisters[P305_NBUS_VOLTAGE] = sGrowattModbusReg_t{
      43, 0, SIZE_16BIT, F("NBusVoltage"), 0.1, 0.1, VOLTAGE, false, false};

  Protocol.InputFragmentCount = 1;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 45};

  Protocol.HoldingRegisterCount = 0;
  Protocol.HoldingFragmentCount = 0;
}
