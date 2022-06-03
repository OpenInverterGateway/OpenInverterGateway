#ifndef _GROWATT_H_
#define _GROWATT_H_

#include "Arduino.h"


// Protocol v3.05 also knows Autostart, v1.05 does not:
typedef enum
{
  GwOnOffOFF    = 0x0000,
  GwOnOffON     = 0x0001,   // default for v1.05
  GwOnOffOFFAuto = 0x0100,
  GwOnOffONAuto = 0x0101    // default for v3.05
}eGrowattOnOff_t;

typedef enum
{
  GwStatusWaiting = 0,
  GwStatusNormal  = 1,
  GwStatusFault   = 3
}eGrowattStatus_t;

typedef enum
{
  Undef_stick  = 0,
  ShineWiFi_S  = 1, // Serial DB9-Connector, 9600Bd, Protocol v3.05 (2013)
  ShineWiFi_X  = 2, // USB Type A with Bajonet locking , 115200Bd, Protocol v1.05 (2018)
  ShineWiFi_F  = 3  // USB Type A DB9-style screws, (Baudrate and protocol unclear; likely 115200Bd / v1.05)
}eDevice_t;


typedef struct
{
  /* 
   *  Allocate the Fields we might use, taken from the Protocol descriptions; 
   *  add "simplified" Fileds for single Input/Output inverters and backwards compatibility, containing a copy of the data 
   *  Roughly in ascending order of its register numbering
   */

  /* 
   *  Data from Holding Registers
   *  Contains mostly static data (Type, Capability, SerialNo, Firmware version)
   *  some may be written to (e.g. Status On/Off, Maximum Power)
   *  It makes sense to read them only once at startup
   */
  eGrowattOnOff_t InverterOnOff;  // see enum for possible states
  uint16_t  u16PmaxActivePercent; // in 0..100  Output ActivePower in Percent of Nominal Power, can be written to
  uint16_t  u16PmaxReactivePercent; // in 0..100  Output ReacivePower in Percent of Nominal Power, can be written to
  uint16_t  u16PwrFactor;         // 0..10000 underexcited, 10000..20000 overexcited, default 10000
  uint32_t  u32NominalPower;      // in 0.1VA   Nominal Design Power (read only)
  uint16_t  u16NominalPvVoltage;  // in 0.1V    Nominal PV Design Voltage
  char      cFirmwareVersion[6];    // 3 Registers with 6 Bytes in ASCII
  char      cFirmwareVersion2[6];   // 3 Registers with 6 Bytes in ASCII
  char      cSerialNumber[10];      // 5 Registers with 10 Bytes in ASCII
  uint16_t  u16ComAddress;        // Modbus Address of the RS485 Communication Interface, 1..127, can be written to
  
  // Some more protocol-specific Holding Registers
  char      ManufacturerInfo[16];   // 8 Registers with 16 Bytes on ASCII (Protocol v1.05) "   PV Inverter  "
  uint16_t  u16DeviceTypeCode;    // See Device Type Code List in Note &*6 in Protocol v1.05; not mentioned in v3.05, may be helpful in Device Autodetecton
  uint16_t  u16TrackerPhaseNum;   // Amount of PV-Trackers (High Byte, 1..8) and AC-Phases (Low Byte, 1..3), Protocol v1.05
  uint16_t  u16ModbusVersion;     // Modbus Protocol Version, in 0.01; - Reg 73 reads '305' in Protocol v3.05; Reg 88 in Protocol v1.05, may be helpful for Protocol Autodetection
  uint16_t  u16TrackerModel;      // PV-Input Tracker working Model: 0=Independent, 1:DC-Source, 2:parallel, can be written to

  /*
   *  Data from Input Registers
   *  Contains volatile operational Data (Voltage, Current, Power, ...)
   *  All are Read-Only
   *  Read them periodically
   */
  eGrowattStatus_t  InverterStatus; 
  uint32_t u32DcPower;            // in 0.1 W   Total Input Power of all PV Strings
  uint16_t u16DcVoltage;          // in 0.1 V   PV Input Voltage for Single-Input (use value from PV1)
  uint16_t u16DcInputCurrent;     // in 0.1 A   PV Input Current for Single-Input (use value from PV1)
  
  // Protocol v3.05 and v1.05 define two PV inputs, on same registers:
  uint16_t u16PV1DcVoltage;       // in 0.1 V   PV Input 1 - Voltage
  uint16_t u16PV1DcInputCurrent;  // in 0.1 A   PV Input 1 - Current
  uint32_t u32PV1DcPower;         // in 0.1 W   PV Input 1 - Power
  uint16_t u16PV2DcVoltage;       // in 0.1 V   PV Input 2 - Voltage
  uint16_t u16PV2DcInputCurrent;  // in 0.1 A   PV Input 2 - Current
  uint32_t u32PV2DcPower;         // in 0.1 W   PV Input 2 - Power
  
  // Protocol v1.05 defines up to 8 PV Inputs:
  uint16_t u16PV3DcVoltage;       // in 0.1 V   PV Input 3 - Voltage
  uint16_t u16PV3DcInputCurrent;  // in 0.1 A   PV Input 3 - Current
  uint32_t u32PV3DcPower;         // in 0.1 W   PV Input 3 - Power
  uint16_t u16PV4DcVoltage;       // in 0.1 V   PV Input 4 - Voltage
  uint16_t u16PV4DcInputCurrent;  // in 0.1 A   PV Input 4 - Current
  uint32_t u32PV4DcPower;         // in 0.1 W   PV Input 4 - Power
  uint16_t u16PV5DcVoltage;       // in 0.1 V   PV Input 5 - Voltage
  uint16_t u16PV5DcInputCurrent;  // in 0.1 A   PV Input 5 - Current
  uint32_t u32PV5DcPower;         // in 0.1 W   PV Input 5 - Power
  uint16_t u16PV6DcVoltage;       // in 0.1 V   PV Input 6 - Voltage
  uint16_t u16PV6DcInputCurrent;  // in 0.1 A   PV Input 6 - Current
  uint32_t u32PV6DcPower;         // in 0.1 W   PV Input 6 - Power
  uint16_t u16PV7DcVoltage;       // in 0.1 V   PV Input 7 - Voltage
  uint16_t u16PV7DcInputCurrent;  // in 0.1 A   PV Input 7 - Current
  uint32_t u32PV7DcPower;         // in 0.1 W   PV Input 7 - Power
  uint16_t u16PV8DcVoltage;       // in 0.1 V   PV Input 8 - Voltage
  uint16_t u16PV8DcInputCurrent;  // in 0.1 A   PV Input 8 - Current
  uint32_t u32PV8DcPower;         // in 0.1 W   PV Input 8 - Power

  // Protocol v 3.05 and v1.05 define three-phase system; but using different registers:
  uint32_t u32AcPower;         // in 0.1 W      AC Output Power (total)
  uint16_t u16AcFrequency;     // in 0.01 Hz    AC Grif Frequency

  uint16_t u16AcVoltage;       // in 0.1 V      AC Voltage for Single Phase (use value from Phase 1)
  uint16_t u16AcOutputCurrent; // in 0.1 A      AC Current for Single Phase (use value from Phase 1)

  uint16_t u16Ac1Voltage;       // in 0.1 V     AC Voltage Phase 1
  uint16_t u16Ac1OutputCurrent; // in 0.1 A     AC Current Phase 1
  uint32_t u32Ac1Power;         // in 0.1 W     AC Output Power Phase 1
  uint16_t u16Ac2Voltage;       // in 0.1 V     AC Voltage Phase 2
  uint16_t u16Ac2OutputCurrent; // in 0.1 A     AC Current Phase 2
  uint32_t u32Ac2Power;         // in 0.1 W     AC Output Power Phase 2
  uint16_t u16Ac3Voltage;       // in 0.1 V     AC Voltage Phase 3
  uint16_t u16Ac3OutputCurrent; // in 0.1 A     AC Current Phase 3
  uint32_t u32Ac3Power;         // in 0.1 W     AC Output Power Phase 3

  uint32_t u32EnergyToday;     // in 0.1 kWh
  uint32_t u32EnergyTotal;     // in 0.1 kWh
  uint32_t u32OperatingTime;   // in 0.5 s
  uint16_t u16Temperature;     // 0.1 degree celsius
  uint16_t u16PwrLimit;        // %
  uint16_t u16EnExportLimit;   // 0.1 %
  uint16_t u16ExportPwrLimit;  // 0.1 %
  uint16_t u16ExportFaultLimit;// 0.1 %
}sGrowattData_t;


class Growatt
{
  public:
    Growatt();
    void begin(Stream &serial);
    bool UpdateData();
    bool ReadHoldingReg(uint16_t adr, uint16_t* result);
    bool WriteHoldingReg(uint16_t adr, uint16_t value);
    bool ReadInputReg(uint16_t adr, uint16_t* result);
   
    eDevice_t        GetWiFiStickType();
    eGrowattStatus_t GetStatus();
    float            GetDcPower();
    float            GetDcVoltage();
    float            GetDcInputCurrent();
    float            GetAcFrequency();
    float            GetAcVoltage();
    float            GetAcOutputCurrent();
    float            GetAcPower();
    float            GetEnergyToday();
    float            GetEnergyTotal();
    uint32           GetOperatingTime();
    float            GetInverterTemperature();
    uint16_t         GetPwrLimit();
    uint16_t         GetEnExportLimit();
    uint16_t         GetExportPwrLimit();
    uint16_t         GetExportFaultLimit();    
  private:
    sGrowattData_t _Data;
    eDevice_t      _eDevice;

};

#endif // _GROWATT_H_
