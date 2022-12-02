#include "Arduino.h"

#include "Growatt120.h"

// Supported inverters:
// - Growatt MIC 1000TL-X
// - Growatt MID 30kTL3-X

// Should also work for the following series (with input register range 0~124):
// - TL-X（MIN Type）
// - TL3-X(MAX, MID, MAC Type)
// - Storage(MIX Type)
// - Storage(SPA Type)
// - Storage(SPH Type)


void init_growatt120(sProtocolDefinition_t &Protocol)
{
    // definition of input registers
    Protocol.InputRegisterCount = eP120InputRegisters_t::LASTInput;
    Protocol.InputFragmentCount = 3;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 50};
    Protocol.InputReadFragments[1] = sGrowattReadFragment_t{50, 50};
    Protocol.InputReadFragments[2] = sGrowattReadFragment_t{100, 6};

    // FRAGMENT 1: BEGIN
    // address, value, size, name, multiplier, unit, frontend, plot

// 0. Inverter Status Inverter run state
    Protocol.InputRegisters[P120_I_STATUS] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "InverterStatus", 1, NONE, true, false};    // 0:waiting, 1:normal, 3:fault
// 1. Ppv H Input power (high) 0.1W
    Protocol.InputRegisters[P120_INPUT_POWER] = sGrowattModbusReg_t{1, 0, SIZE_32BIT, "InputPower", 0.1, POWER_W, true, true};
// 2. Ppv L Input power (low) 0.1W
// 3. Vpv1 PV1 voltage 0.1V
    Protocol.InputRegisters[P120_PV1_VOLTAGE] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "PV1Voltage", 0.1, VOLTAGE, false, false};
// 4. PV1Curr PV1 input current 0.1A
    Protocol.InputRegisters[P120_PV1_INPUT_CURRENT] = sGrowattModbusReg_t{4, 0, SIZE_16BIT, "PV1InputCurrent", 0.1, CURRENT, false, false};
// 5. Ppv1 H PV1 input power(high) 0.1W
    Protocol.InputRegisters[P120_PV1_INPUT_POWER] = sGrowattModbusReg_t{5, 0, SIZE_32BIT, "PV1InputPower", 0.1, POWER_W, true, true};
// 6. Ppv1 L PV1 input power(low) 0.1W
// 7. Vpv2 PV2 voltage 0.1V
    Protocol.InputRegisters[P120_PV2_VOLTAGE] = sGrowattModbusReg_t{7, 0, SIZE_16BIT, "PV2Voltage", 0.1, VOLTAGE, false, false};
// 8. PV2Curr PV2 input current 0.1A
    Protocol.InputRegisters[P120_PV2_INPUT_CURRENT] = sGrowattModbusReg_t{8, 0, SIZE_16BIT, "PV2InputCurrent", 0.1, CURRENT, false, false};
// 9. Ppv2 H PV2 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV2_INPUT_POWER] = sGrowattModbusReg_t{9, 0, SIZE_32BIT, "PV2InputPower", 0.1, POWER_W, true, true};
// 10. Ppv2 L PV2 input power (low) 0.1W
// 11. Vpv3 PV3 voltage 0.1V
    Protocol.InputRegisters[P120_PV3_VOLTAGE] = sGrowattModbusReg_t{11, 0, SIZE_16BIT, "PV3Voltage", 0.1, VOLTAGE, false, false};
// 12. PV3Curr PV3 input current 0.1A
    Protocol.InputRegisters[P120_PV3_INPUT_CURRENT] = sGrowattModbusReg_t{12, 0, SIZE_16BIT, "PV3InputCurrent", 0.1, CURRENT, false, false};
// 13. Ppv3 H PV3 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV3_INPUT_POWER] = sGrowattModbusReg_t{13, 0, SIZE_32BIT, "PV3InputPower", 0.1, POWER_W, true, true};
// 14. Ppv3 L PV3 input power (low) 0.1W
// 15. Vpv4 PV4 voltage 0.1V
    Protocol.InputRegisters[P120_PV4_VOLTAGE] = sGrowattModbusReg_t{15, 0, SIZE_16BIT, "PV4Voltage", 0.1, VOLTAGE, false, false};
// 16. PV4Curr PV4 input current 0.1A
    Protocol.InputRegisters[P120_PV4_INPUT_CURRENT] = sGrowattModbusReg_t{16, 0, SIZE_16BIT, "PV4InputCurrent", 0.1, CURRENT, false, false};
// 17. Ppv4 H PV4 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV4_INPUT_POWER] = sGrowattModbusReg_t{17, 0, SIZE_32BIT, "PV4InputPower", 0.1, POWER_W, true, true};
// 18. Ppv4 L PV4 input power (low) 0.1W
// 19. Vpv5 PV5 voltage 0.1V
    Protocol.InputRegisters[P120_PV5_VOLTAGE] = sGrowattModbusReg_t{19, 0, SIZE_16BIT, "PV5Voltage", 0.1, VOLTAGE, false, false};
// 20. PV5Curr PV5 input current 0.1A
    Protocol.InputRegisters[P120_PV5_INPUT_CURRENT] = sGrowattModbusReg_t{20, 0, SIZE_16BIT, "PV5InputCurrent", 0.1, CURRENT, false, false};
// 21. Ppv5H PV5 input power(high) 0.1W
    Protocol.InputRegisters[P120_PV5_INPUT_POWER] = sGrowattModbusReg_t{21, 0, SIZE_32BIT, "PV5InputPower", 0.1, POWER_W, true, true};
// 22. Ppv5 L PV5 input power(low) 0.1W
// 23. Vpv6 PV6 voltage 0.1V
    Protocol.InputRegisters[P120_PV6_VOLTAGE] = sGrowattModbusReg_t{23, 0, SIZE_16BIT, "PV6Voltage", 0.1, VOLTAGE, false, false};
// 24. PV6Curr PV6 input current 0.1A
    Protocol.InputRegisters[P120_PV6_INPUT_CURRENT] = sGrowattModbusReg_t{24, 0, SIZE_16BIT, "PV6InputCurrent", 0.1, CURRENT, false, false};
// 25. Ppv6 H PV6 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV6_INPUT_POWER] = sGrowattModbusReg_t{25, 0, SIZE_32BIT, "PV6InputPower", 0.1, POWER_W, true, true};
// 26. Ppv6 L PV6 input power (low) 0.1W
// 27. Vpv7 PV7 voltage 0.1V
    Protocol.InputRegisters[P120_PV7_VOLTAGE] = sGrowattModbusReg_t{27, 0, SIZE_16BIT, "PV7Voltage", 0.1, VOLTAGE, false, false};
// 28. PV7Curr PV7 input current 0.1A
    Protocol.InputRegisters[P120_PV7_INPUT_CURRENT] = sGrowattModbusReg_t{28, 0, SIZE_16BIT, "PV7InputCurrent", 0.1, CURRENT, false, false};
// 29. Ppv7 H PV7 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV7_INPUT_POWER] = sGrowattModbusReg_t{29, 0, SIZE_32BIT, "PV7InputPower", 0.1, POWER_W, true, true};
// 30. Ppv7 L PV7 input power (low) 0.1W
// 31. Vpv8 PV8 voltage 0.1V
    Protocol.InputRegisters[P120_PV8_VOLTAGE] = sGrowattModbusReg_t{31, 0, SIZE_16BIT, "PV8Voltage", 0.1, VOLTAGE, false, false};
// 32. PV8Curr PV8 input current 0.1A
    Protocol.InputRegisters[P120_PV8_INPUT_CURRENT] = sGrowattModbusReg_t{32, 0, SIZE_16BIT, "PV8InputCurrent", 0.1, CURRENT, false, false};
// 33. Ppv8 H PV8 input power (high) 0.1W
    Protocol.InputRegisters[P120_PV8_INPUT_POWER] = sGrowattModbusReg_t{33, 0, SIZE_32BIT, "PV8InputPower", 0.1, POWER_W, true, true};
// 34. Ppv8 L PV8 input power (low) 0.1W
// 35. Pac H Output power (high) 0.1W
    Protocol.InputRegisters[P120_OUTPUT_POWER] = sGrowattModbusReg_t{35, 0, SIZE_32BIT, "OutputPower", 0.1, POWER_W, true, true};
// 36. Pac L Output power (low) 0.1W
// 37. Fac Grid frequency 0.01Hz
    Protocol.InputRegisters[P120_GRID_FREQUENCY] = sGrowattModbusReg_t{37, 0, SIZE_16BIT, "GridFrequency", 0.01, FREQUENCY, false, false};
// 38. Vac1 Three/single phase grid voltage 0.1V
    Protocol.InputRegisters[P120_GRID_L1_VOLTAGE] = sGrowattModbusReg_t{38, 0, SIZE_16BIT, "GridL1Voltage", 0.1, VOLTAGE, false, false};
// 39. Iac1 Three/single phase grid output current 0.1A
    Protocol.InputRegisters[P120_GRID_L1_OUTPUT_CURRENT] = sGrowattModbusReg_t{39, 0, SIZE_16BIT, "GridL1OutputCurrent", 0.1, CURRENT, false, false};
// 40. Pac1 H Three/single phase grid output watt(high) 0.1VA
    Protocol.InputRegisters[P120_GRID_L1_OUTPUT_POWER] = sGrowattModbusReg_t{40, 0, SIZE_32BIT, "GridL1OutputPower", 0.1, VA, false, false};
// 41. Pac1 L Three/single phase grid output watt(low) 0.1VA
// 42. Vac2 Three phase grid voltage 0.1V
    Protocol.InputRegisters[P120_GRID_L2_VOLTAGE] = sGrowattModbusReg_t{42, 0, SIZE_16BIT, "GridL2Voltage", 0.1, VOLTAGE, false, false};
// 43. Iac2 Three phase grid output current 0.1A
    Protocol.InputRegisters[P120_GRID_L2_OUTPUT_CURRENT] = sGrowattModbusReg_t{43, 0, SIZE_16BIT, "GridL2OutputCurrent", 0.1, CURRENT, false, false};
// 44. Pac2 H Three phase grid output power (high) 0.1VA
    Protocol.InputRegisters[P120_GRID_L2_OUTPUT_POWER] = sGrowattModbusReg_t{44, 0, SIZE_32BIT, "GridL2OutputPower", 0.1, VA, false, false};
// 45. Pac2 L Three phase grid output power (low) 0.1VA
// 46. Vac3 Three phase grid voltage 0.1V
    Protocol.InputRegisters[P120_GRID_L3_VOLTAGE] = sGrowattModbusReg_t{46, 0, SIZE_16BIT, "GridL3Voltage", 0.1, VOLTAGE, false, false};
// 47. Iac3 Three phase grid output current 0.1A
    Protocol.InputRegisters[P120_GRID_L3_OUTPUT_CURRENT] = sGrowattModbusReg_t{47, 0, SIZE_16BIT, "GridL3OutputCurrent", 0.1, CURRENT, false, false};
// 48. Pac3 H Three phase grid output power (high) 0.1VA
    Protocol.InputRegisters[P120_GRID_L3_OUTPUT_POWER] = sGrowattModbusReg_t{48, 0, SIZE_32BIT, "GridL3OutputPower", 0.1, VA, false, false};
    // FRAGMENT 1: END

    // FRAGMENT 2: BEGIN
// 49. Pac3 L Three phase grid output power (low) 0.1VA
// 50. Vac_RS Three phase grid voltage 0.1V Line voltage
    Protocol.InputRegisters[P120_GRID_RS_VOLTAGE] = sGrowattModbusReg_t{50, 0, SIZE_16BIT, "GridRSVoltage", 0.1, VOLTAGE, false, false};
// 51. Vac_ST Three phase grid voltage 0.1V Line voltage
    Protocol.InputRegisters[P120_GRID_ST_VOLTAGE] = sGrowattModbusReg_t{51, 0, SIZE_16BIT, "GridSTVoltage", 0.1, VOLTAGE, false, false};
// 52. Vac_TR Three phase grid voltage 0.1V Line voltage
    Protocol.InputRegisters[P120_GRID_TR_VOLTAGE] = sGrowattModbusReg_t{52, 0, SIZE_16BIT, "GridTRVoltage", 0.1, VOLTAGE, false, false};
// 53. Eac today H Today generate energy (high) 0.1kWH
    Protocol.InputRegisters[P120_ENERGY_TODAY] = sGrowattModbusReg_t{53, 0, SIZE_32BIT, "EnergyToday", 0.1, POWER_KWH, true, false};
// 54. Eac today L Today generate energy (low) 0.1kWH
// 55. Eac total H Total generate energy (high) 0.1kWH
    Protocol.InputRegisters[P120_ENERGY_TOTAL] = sGrowattModbusReg_t{55, 0, SIZE_32BIT, "EnergyTotal", 0.1, POWER_KWH, true, false};
// 56. Eac total L Total generate energy (low) 0.1kWH
// 57. Time total H Work time total (high) 0.5s
    Protocol.InputRegisters[P120_WORK_TIME_TOTAL] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "WorkTimeTotal", 0.5, SECONDS, false, false};
// 58. Time total L Work time total (low) 0.5s
// 59. Epv1_today H PV1 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV1_ENERGY_TODAY] = sGrowattModbusReg_t{59, 0, SIZE_32BIT, "PV1EnergyToday", 0.1, POWER_KWH, false, false};
// 60. Epv1_today L PV1 Energy today (low) 0.1kWh
// 61. Epv1_total H PV1 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV1_ENERGY_TOTAL] = sGrowattModbusReg_t{61, 0, SIZE_32BIT, "PV1EnergyTotal", 0.1, POWER_KWH, false, false};
// 62. Epv1_total L PV1 Energy total (low) 0.1kWh
// 63. Epv2_today H PV2 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV2_ENERGY_TODAY] = sGrowattModbusReg_t{63, 0, SIZE_32BIT, "PV2EnergyToday", 0.1, POWER_KWH, false, false};
// 64. Epv2_today L PV2 Energy today (low) 0.1kWh
// 65. Epv2_total H PV2 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV2_ENERGY_TOTAL] = sGrowattModbusReg_t{65, 0, SIZE_32BIT, "PV2EnergyTotal", 0.1, POWER_KWH, false, false};
// 66. Epv2_total L PV2 Energy total (low) 0.1kWh
// 67. Epv3_today H PV3 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV3_ENERGY_TODAY] = sGrowattModbusReg_t{67, 0, SIZE_32BIT, "PV3EnergyToday", 0.1, POWER_KWH, false, false};
// 68. Epv3_today L PV3 Energy today (low) 0.1kWh
// 69. Epv3_total H PV3 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV3_ENERGY_TOTAL] = sGrowattModbusReg_t{69, 0, SIZE_32BIT, "PV3EnergyTotal", 0.1, POWER_KWH, false, false};
// 70. Epv3_total L PV3 Energy total (low) 0.1kWh
// 71. Epv4_today H PV4 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV4_ENERGY_TODAY] = sGrowattModbusReg_t{71, 0, SIZE_32BIT, "PV4EnergyToday", 0.1, POWER_KWH, false, false};
// 72. Epv4_today L PV4 Energy today (low) 0.1kWh
// 73. Epv4_total H PV4 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV4_ENERGY_TOTAL] = sGrowattModbusReg_t{73, 0, SIZE_32BIT, "PV4EnergyTotal", 0.1, POWER_KWH, false, false};
// 74. Epv4_total L PV4 Energy total (low) 0.1kWh
// 75. Epv5_today H PV5 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV5_ENERGY_TODAY] = sGrowattModbusReg_t{75, 0, SIZE_32BIT, "PV5EnergyToday", 0.1, POWER_KWH, false, false};
// 76. Epv5_today L PV5 Energy today (low) 0.1kWh
// 77. Epv5_total H PV5 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV5_ENERGY_TOTAL] = sGrowattModbusReg_t{77, 0, SIZE_32BIT, "PV5EnergyTotal", 0.1, POWER_KWH, false, false};
// 78. Epv5_total L PV5 Energy total (low) 0.1kWh
// 79. Epv6_today H PV6 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV6_ENERGY_TODAY] = sGrowattModbusReg_t{79, 0, SIZE_32BIT, "PV6EnergyToday", 0.1, POWER_KWH, false, false};
// 80. Epv6_today L PV6Energy today (low) 0.1kWh
// 81. Epv6_total H PV6 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV6_ENERGY_TOTAL] = sGrowattModbusReg_t{81, 0, SIZE_32BIT, "PV6EnergyTotal", 0.1, POWER_KWH, false, false};
// 82. Epv6_total L PV6 Energy total (low) 0.1kWh
// 83. Epv7_today H PV7 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV7_ENERGY_TODAY] = sGrowattModbusReg_t{83, 0, SIZE_32BIT, "PV7EnergyToday", 0.1, POWER_KWH, false, false};
// 84. Epv7_today L PV7 Energy today (low) 0.1kWh
// 85. Epv7_total H PV7 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV7_ENERGY_TOTAL] = sGrowattModbusReg_t{85, 0, SIZE_32BIT, "PV7EnergyTotal", 0.1, POWER_KWH, false, false};
// 86. Epv7_total L PV7 Energy total (low) 0.1kWh
// 87. Epv8_today H PV8 Energy today (high) 0.1kWh
    Protocol.InputRegisters[P120_PV8_ENERGY_TODAY] = sGrowattModbusReg_t{87, 0, SIZE_32BIT, "PV8EnergyToday", 0.1, POWER_KWH, false, false};
// 88. Epv8_today L PV8Energy today (low) 0.1kWh
// 89. Epv8_total H PV8 Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV8_ENERGY_TOTAL] = sGrowattModbusReg_t{89, 0, SIZE_32BIT, "PV8EnergyTotal", 0.1, POWER_KWH, false, false};
// 90. Epv8_total L PV8 Energy total (low) 0.1kWh
// 91. Epv_total H PV Energy total (high) 0.1kWh
    Protocol.InputRegisters[P120_PV_ENERGY_TOTAL] = sGrowattModbusReg_t{91, 0, SIZE_32BIT, "PVEnergyTotal", 0.1, POWER_KWH, true, false};
// 92. Epv_total L PV Energy total (low) 0.1kWh
// 93. Temp1 Inverter temperature 0.1C
    Protocol.InputRegisters[P120_INVERTER_TEMPERATURE] = sGrowattModbusReg_t{93, 0, SIZE_16BIT, "InverterTemperature", 0.1, TEMPERATURE, true, true};
// 94. Temp2 The inside IPM in inverter Temperature 0.1C
    Protocol.InputRegisters[P120_INVERTER_IPM_TEMPERATURE] = sGrowattModbusReg_t{94, 0, SIZE_16BIT, "InverterIPMTemperature", 0.1, TEMPERATURE, false, false};
// 95. Temp3 Boost temperature 0.1C
    Protocol.InputRegisters[P120_INVERTER_BOOST_TEMPERATURE] = sGrowattModbusReg_t{95, 0, SIZE_16BIT, "InverterBoostTemperature", 0.1, TEMPERATURE, false, false};
// 96. Temp4 reserved
// 97. Temp5 reserved
// 98. P Bus Voltage P Bus inside Voltage 0.1V
    Protocol.InputRegisters[P120_BUS_P_VOLTAGE] = sGrowattModbusReg_t{98, 0, SIZE_16BIT, "BusPVoltage", 0.1, VOLTAGE, false, false};
// 99. N Bus Voltage N Bus inside Voltage 0.1V
    Protocol.InputRegisters[P120_BUS_N_VOLTAGE] = sGrowattModbusReg_t{99, 0, SIZE_16BIT, "BusNVoltage", 0.1, VOLTAGE, false, false};
    // FRAGMENT 2: END

    // FRAGMENT 3: BEGIN
// 100. IPF Inverter output PF now 0-20000
// 101. RealOPPercent Real Output power Percent 1%
    Protocol.InputRegisters[P120_REAL_OUTPUT_POWER] = sGrowattModbusReg_t{101, 0, SIZE_16BIT, "RealOutputPowerPercent", 1, PRECENTAGE, false, false};
// 102. OPFullwatt H Output Maxpower Limited high
    Protocol.InputRegisters[P120_OUTPUT_MAXPOWER_LIMITED] = sGrowattModbusReg_t{102, 0, SIZE_16BIT, "LimitedOutputPower", 1, POWER_W, false, false};
// 103. OPFullwatt L Output Maxpower Limited low 0.1W
// 104. DeratingMode DeratingMode // 0:no derate; 1:PV; 2:*; 3:Vac; 4:Fac; 5:Tboost; 6:Tinv; 7:Control; 8:*; 9:*OverBack ByTime; “*”is Reserved
    Protocol.InputRegisters[P120_DERATINGMODE] = sGrowattModbusReg_t{104, 0, SIZE_16BIT, "DeratingMode", 1, NONE, false, false};
// 105. Fault code Inverter fault code &*1
    Protocol.InputRegisters[P120_FAULT_CODE] = sGrowattModbusReg_t{105, 0, SIZE_16BIT, "FaultCode", 1, NONE, true, false};    // 0:no derate; 1:PV; 2:*; 3:Vac; 4:Fac; 5:Tboost; 6:Tinv; 7:Control; 8:*; 9:*OverBack ByTime; “*”is Reserved
// 106. Fault Bitcode H Inverter fault code high &*8
// 107. Fault Bitcode L Inverter fault code low
// 108. Fault Bit_II H Inverter fault code_II high --预留 mix，
// 109. Fault Bit_II L Inverter fault code_II low 待定义
// 110. Warning bit H Warning bit H &*8
// 111. Warning bit L Warning bit L
// 112. bINVWarnCode bINVWarnCode
// 113. real Power Percent real Power Percent 0-100 %
    // Protocol.InputRegisters[P120_REAL_POWER] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "RealPowerPercent", 1, PRECENTAGE, false, false};
// 114. inv start delay time inv start delay time
    // Protocol.InputRegisters[P120_START_DELAY] = sGrowattModbusReg_t{57, 0, SIZE_32BIT, "StartDelay", 0.5, SECONDS, false, false};
// 115. bINVAllFaultCod e bINVAllFaultCode
    // FRAGMENT 3: END


    // definition of holding registers
    Protocol.HoldingRegisterCount = eP120HoldingRegisters_t::LASTHolding;
    Protocol.HoldingFragmentCount = 1;
    Protocol.HoldingReadFragments[0] = sGrowattReadFragment_t{0, 4};

    // FRAGMENT 1: BEGIN
    Protocol.HoldingRegisters[P120_OnOff] = sGrowattModbusReg_t{0, 0, SIZE_16BIT, "OnOff", 1, NONE, true, false};
    Protocol.HoldingRegisters[P120_CMD_MEMORY_STATE] = sGrowattModbusReg_t{2, 0, SIZE_16BIT, "CmdMemoryState", 1, NONE, true, false};
    Protocol.HoldingRegisters[P120_Active_P_Rate] = sGrowattModbusReg_t{3, 0, SIZE_16BIT, "ActivePowerRate", 1, PRECENTAGE, true, false};
    // FRAGMENT 1: END
}
