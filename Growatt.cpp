#include <ModbusMaster.h>

#include "Growatt.h"

ModbusMaster Modbus;

// Constructor
Growatt::Growatt(void)
{

}

void Growatt::begin(Stream &serial)
{
  Modbus.begin(1, serial);
}

bool Growatt::UpdateData()
{
  uint8_t res;
    
  res = Modbus.readInputRegisters(0, 33);
  
  if (res == Modbus.ku8MBSuccess)
  {
    _Data.InverterStatus     =  (eGrowattStatus)Modbus.getResponseBuffer(0);
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
    _Data.u16Temperaure      =  Modbus.getResponseBuffer(32);

    return true;
  }else{
    return false;
  }
}

eGrowattStatus Growatt::GetStatus()
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
  return ((float)_Data.u16Temperaure)/10.0;
}




