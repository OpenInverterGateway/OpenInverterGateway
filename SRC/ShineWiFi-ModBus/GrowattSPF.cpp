#include "Arduino.h"

#include "GrowattSPF.h"

/* Tested on Growatt SPF5000ES
   Replacing ShineWifi-F "USB" stick
*/

void init_growattSPF(sProtocolDefinition_t &Protocol) {
  // definition of input registers
  Protocol.InputRegisterCount = 27;
  // address, value, size, name, multiplier, unit, frontend, plot
  Protocol.InputRegisters[SPF_I_STATUS] = sGrowattModbusReg_t{
      0, 0, SIZE_16BIT, "InverterStatus", 1, 1, NONE, true, false};  // #1
  Protocol.InputRegisters[SPF_PV1_V] = sGrowattModbusReg_t{
      1, 0, SIZE_16BIT, "PV1Voltage", 0.1, 0.1, VOLTAGE, true, false};  // #2
  Protocol.InputRegisters[SPF_PV2_V] = sGrowattModbusReg_t{
      2, 0, SIZE_16BIT, "PV2Voltage", 0.1, 0.1, VOLTAGE, true, false};  // #3
  Protocol.InputRegisters[SPF_PV1_CHGW] = sGrowattModbusReg_t{
      3, 0, SIZE_32BIT, "PV1ChargePwr", 0.1, 0.1, POWER_W, true, false};  // #4
  Protocol.InputRegisters[SPF_PV2_CHGW] = sGrowattModbusReg_t{
      5, 0, SIZE_32BIT, "PV2ChargePwr", 0.1, 0.1, POWER_W, true, false};  // #5
  Protocol.InputRegisters[SPF_BUCK1_I] = sGrowattModbusReg_t{
      7, 0, SIZE_16BIT, "Buck1Current", 0.1, 0.1, CURRENT, true, false};  // #6
  Protocol.InputRegisters[SPF_BUCK2_I] = sGrowattModbusReg_t{
      8, 0, SIZE_16BIT, "Buck2Current", 0.1, 0.1, CURRENT, true, false};  // #7
  Protocol.InputRegisters[SPF_OUT_PWR] = sGrowattModbusReg_t{
      9, 0, SIZE_32BIT, "OutActivePwr", 0.1, 0.1, POWER_W, true, true};  // #8
  Protocol.InputRegisters[SPF_OUT_VA] = sGrowattModbusReg_t{
      11, 0, SIZE_32BIT, "OutVA", 0.1, 0.1, VA, true, false};  // #9
  Protocol.InputRegisters[SPF_AC_CHGPWR] = sGrowattModbusReg_t{
      13, 0, SIZE_32BIT, "ACChargePwr", 0.1, 0.1, POWER_W, true, true};  // #10
  Protocol.InputRegisters[SPF_AC_CHGVA] = sGrowattModbusReg_t{
      15, 0, SIZE_32BIT, "ACChargeVA", 0.1, 0.1, VA, true, false};  // #11
  Protocol.InputRegisters[SPF_BATT_V] =
      sGrowattModbusReg_t{17,   0,       SIZE_16BIT, "BattVoltage", 0.01,
                          0.01, VOLTAGE, true,       false};  // #12
  Protocol.InputRegisters[SPF_BATT_SOC] = sGrowattModbusReg_t{
      18, 0, SIZE_16BIT, "BattSOC", 1, 1, PRECENTAGE, true, false};  // #13
  Protocol.InputRegisters[SPF_BUS_V] = sGrowattModbusReg_t{
      19, 0, SIZE_16BIT, "BusVoltage", 0.1, 0.1, VOLTAGE, true, false};  // #14
  Protocol.InputRegisters[SPF_GRID_V] =
      sGrowattModbusReg_t{20,      0,    SIZE_16BIT, "GridInVoltage", 0.1, 0.1,
                          VOLTAGE, true, false};  // #15
  Protocol.InputRegisters[SPF_LINE_F] = sGrowattModbusReg_t{
      21,        0,    SIZE_16BIT, "LineFrequency", 0.01, 0.01,
      FREQUENCY, true, false};  // #16
  Protocol.InputRegisters[SPF_OUT_V] = sGrowattModbusReg_t{
      22, 0, SIZE_16BIT, "OutVoltage", 0.1, 0.1, VOLTAGE, true, false};  // #17
  Protocol.InputRegisters[SPF_OUT_F] =
      sGrowattModbusReg_t{23,   0,         SIZE_16BIT, "OutFrequency", 0.01,
                          0.01, FREQUENCY, true,       false};  // #18
  Protocol.InputRegisters[SPF_OUT_DCV] =
      sGrowattModbusReg_t{24,  0,       SIZE_16BIT, "OutDCVoltage", 0.1,
                          0.1, VOLTAGE, true,       false};  // #19
  Protocol.InputRegisters[SPF_INV_T] =
      sGrowattModbusReg_t{25,  0,           SIZE_16BIT, "InverterTemp", 0.1,
                          0.1, TEMPERATURE, true,       false};  // #20
  Protocol.InputRegisters[SPF_DCDC_T] =
      sGrowattModbusReg_t{26,  0,           SIZE_16BIT, "DCDCTemp", 0.1,
                          0.1, TEMPERATURE, true,       false};  // #21
  Protocol.InputRegisters[SPF_LOAD] =
      sGrowattModbusReg_t{27,  0,          SIZE_16BIT, "LoadPercent", 0.1,
                          0.1, PRECENTAGE, true,       false};  // #22
  Protocol.InputRegisters[SPF_BUCK1_T] =
      sGrowattModbusReg_t{32,  0,           SIZE_16BIT, "Buck1Temp", 0.1,
                          0.1, TEMPERATURE, true,       false};  // #23
  Protocol.InputRegisters[SPF_BUCK2_T] =
      sGrowattModbusReg_t{33,  0,           SIZE_16BIT, "Buck2Temp", 0.1,
                          0.1, TEMPERATURE, true,       false};  // #24
  Protocol.InputRegisters[SPF_AC_INPWR] = sGrowattModbusReg_t{
      36, 0, SIZE_32BIT, "ACInPwr", 0.1, 0.1, POWER_W, true, true};  // #25
  Protocol.InputRegisters[SPF_AC_INVA] = sGrowattModbusReg_t{
      38, 0, SIZE_32BIT, "ACInVA", 0.1, 0.1, VA, true, false};  // #26
  Protocol.InputRegisters[SPF_BATT_PWR] = sGrowattModbusReg_t{
      77, 0, SIZE_32BIT, "BattPwr", 0.1, 0.1, POWER_W, true, false};  // #27

  Protocol.InputFragmentCount = 2;
  Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 40};
  Protocol.InputReadFragments[1] = sGrowattReadFragment_t{77, 79};

  Protocol.HoldingRegisterCount = 0;
  Protocol.HoldingFragmentCount = 0;
}
