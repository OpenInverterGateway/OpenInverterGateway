#include <ModbusMaster.h>

#include "Growatt.h"

ModbusMaster Modbus;

// Constructor
Growatt::Growatt(void)
{

}

void Growatt::begin(Stream &serial)
{
  uint8_t res;
  uint32_t u32EnergyTotal;

  _eDevice = Undef_stick;

  Serial.begin(9600);
  Modbus.begin(1, serial);
  res = Modbus.readInputRegisters(28, 2); // dummy read total energy
  if( res == Modbus.ku8MBSuccess )
  {
    _eDevice = ShineWiFi_S; // Serial
  }
  else
  {
    delay(1000);
    Serial.begin(115200);
    Modbus.begin(1, serial);
    res = Modbus.readInputRegisters(55, 2); // dummy read total energy
    if( res == Modbus.ku8MBSuccess)
    {
      _eDevice = ShineWiFi_X; // USB
    }
  }  
}

bool Growatt::UpdateData()
{
  uint8_t res;

  if( _eDevice == ShineWiFi_S ) // Serial
  {
    res = Modbus.readInputRegisters(0, 33);
    
    if (res == Modbus.ku8MBSuccess)
    {
      _Data.InverterStatus     = (eGrowattStatus_t)Modbus.getResponseBuffer(0);
      _Data.u32DcPower         = (Modbus.getResponseBuffer(1) << 16) + Modbus.getResponseBuffer(2);
      _Data.u16DcVoltage       =  Modbus.getResponseBuffer(3);
      _Data.u16DcInputCurrent  =  Modbus.getResponseBuffer(4);
      _Data.u16AcFrequency     =  Modbus.getResponseBuffer(13);
      _Data.u16AcVoltage       =  Modbus.getResponseBuffer(14);
      _Data.u16AcOutputCurrent =  Modbus.getResponseBuffer(15);
      _Data.u32AcPower         = (Modbus.getResponseBuffer(16) << 16) + Modbus.getResponseBuffer(17);
      _Data.u32EnergyToday     = (Modbus.getResponseBuffer(26) << 16) + Modbus.getResponseBuffer(27);
      _Data.u32EnergyTotal     = (Modbus.getResponseBuffer(28) << 16) + Modbus.getResponseBuffer(29);
      _Data.u32OperatingTime   = (Modbus.getResponseBuffer(30) << 16) + Modbus.getResponseBuffer(31);
      _Data.u16Temperature     =  Modbus.getResponseBuffer(32);
  
      return true;
    }else{
      return false;
    }
  }
  else if( _eDevice == ShineWiFi_X ) // USB
  {
    res = Modbus.readInputRegisters(0, 6);
    if (res == Modbus.ku8MBSuccess)
    {
      _Data.InverterStatus     = (eGrowattStatus_t)Modbus.getResponseBuffer(0);
      _Data.u32DcPower         = (Modbus.getResponseBuffer(5) << 16) + Modbus.getResponseBuffer(6);
      _Data.u16DcVoltage       =  Modbus.getResponseBuffer(3);
      _Data.u16DcInputCurrent  =  Modbus.getResponseBuffer(4);
   
      //return true;
    }else{
      return false;
    }
    
    res = Modbus.readInputRegisters(30, 30);
     
    if (res == Modbus.ku8MBSuccess)
    {
      _Data.u16AcFrequency     =  Modbus.getResponseBuffer(37-30);
      _Data.u16AcVoltage       =  Modbus.getResponseBuffer(38-30);
      _Data.u16AcOutputCurrent =  Modbus.getResponseBuffer(39-30);
      _Data.u32AcPower         = (Modbus.getResponseBuffer(40-30) << 16) + Modbus.getResponseBuffer(41-30);
      _Data.u32EnergyToday     = (Modbus.getResponseBuffer(53-30) << 16) + Modbus.getResponseBuffer(54-30);
      _Data.u32EnergyTotal     = (Modbus.getResponseBuffer(55-30) << 16) + Modbus.getResponseBuffer(56-30);
      _Data.u32OperatingTime   = (Modbus.getResponseBuffer(57-30) << 16) + Modbus.getResponseBuffer(58-30);
   
      //return true;
    }else{
      return false;
    }
    
    res = Modbus.readInputRegisters(3093, 1);
  
    if (res == Modbus.ku8MBSuccess)
    {
      _Data.u16Temperature      =  Modbus.getResponseBuffer(0);
      return true;
    }else{
      return false;
    }
  }
  
  return false;
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint16_t* result)
{
  uint8_t res;
    
  res = Modbus.readHoldingRegisters(adr, 1);
  
  if (res == Modbus.ku8MBSuccess)
  {
    *result = Modbus.getResponseBuffer(0);
    return true;
  }
  else
    return false;
}

bool Growatt::WriteHoldingReg(uint16_t adr, uint16_t value)
{
  uint8_t res;

  res = Modbus.writeSingleRegister(adr, value);
  
  if (res == Modbus.ku8MBSuccess)
  {
    return true;
  }
  else
    return false;
}

eGrowattStatus_t Growatt::GetStatus()
{
  return _Data.InverterStatus;
}

// in 0.1 W
float Growatt::GetDcPower()
{
  return ((float)_Data.u32DcPower)/10.0;
}

// in 0.1 V
float Growatt::GetDcVoltage()
{
  return ((float)_Data.u16DcVoltage)/10.0;
}

// in 0.1 A
float Growatt::GetDcInputCurrent()
{
  return ((float)_Data.u16DcInputCurrent)/10.0;
}

// in 0.1 VA 
float Growatt::GetAcPower()
{
  return ((float)_Data.u32AcPower)/10.0;
}

// in 0.01 Hz 
float Growatt::GetAcFrequency()
{
  return ((float)_Data.u16AcFrequency)/100.0;
}

// in 0.1 V
float Growatt::GetAcVoltage()
{
  return ((float)_Data.u16AcVoltage)/10.0;
}

// in 0.1 A
float Growatt::GetAcOutputCurrent()
{
  return ((float)_Data.u16AcOutputCurrent)/10.0;
}

// in 0.1 kWh
float Growatt::GetEnergyToday()
{
  return ((float)_Data.u32EnergyToday)/10.0;
}

// in 0.1 kWh
float Growatt::GetEnergyTotal()
{
  return ((float)_Data.u32EnergyTotal)/10.0;
}

// in 0.1 s
uint32 Growatt::GetOperatingTime()
{
  return _Data.u32OperatingTime/2;
}

// in 0.1 degree celsius
float Growatt::GetInverterTemperature()
{
  return ((float)_Data.u16Temperature)/10.0;
}

uint16_t Growatt::GetPwrLimit()
{
  return _Data.u16PwrLimit;
}

uint16_t Growatt::GetEnExportLimit()
{
  return _Data.u16EnExportLimit;
}

uint16_t Growatt::GetExportPwrLimit()
{
  return _Data.u16ExportPwrLimit;
}

uint16_t Growatt::GetExportFaultLimit()
{
  return _Data.u16ExportFaultLimit;
}

eDevice_t Growatt::GetWiFiStickType()
{
  return _eDevice;
}
