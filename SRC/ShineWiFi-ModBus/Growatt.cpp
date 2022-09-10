#include <ArduinoJson.h>
#include <ModbusMaster.h>

#include "Config.h"
#include "Growatt.h"
#include "GrowattTypes.h"

#if GROWATT_MODBUS_VERSION == 120
    #include "Growatt120.h"
#elif GROWATT_MODBUS_VERSION == 124
    #include "Growatt124.h"
#elif GROWATT_MODBUS_VERSION == 305
    #include "Growatt305.h"
#else
    #error "Unsupported Growatt Modbus version"
#endif

ModbusMaster Modbus;

// Constructor
Growatt::Growatt()
{
    _eDevice = Undef_stick;
    _PacketCnt = 0;
}

void Growatt::InitProtocol()
{
/**
 * @brief Initialize the protocol struct
 * @param version The version of the modbus protocol to use
 */
#if GROWATT_MODBUS_VERSION == 120
    init_growatt120(_Protocol);
#elif GROWATT_MODBUS_VERSION == 124
    init_growatt124(_Protocol);
#elif GROWATT_MODBUS_VERSION == 305
    init_growatt305(_Protocol);
#else
    #error "Unsupported Growatt Modbus version"
#endif
}

void Growatt::begin(Stream& serial)
{
    /**
     * @brief Set up communication with the inverter
     * @param serial The serial interface
     */
    uint8_t res;

#if SIMULATE_INVERTER == 1
    _eDevice = SIMULATE_DEVICE;
#else
    // init communication with the inverter
    Serial.begin(9600);
    Modbus.begin(1, serial);
    res = Modbus.readInputRegisters(0, 1);
    if (res == Modbus.ku8MBSuccess)
    {
        _eDevice = ShineWiFi_S; // Serial
    }
    else
    {
        delay(1000);
        Serial.begin(115200);
        Modbus.begin(1, serial);
        res = Modbus.readInputRegisters(0, 1);
        if (res == Modbus.ku8MBSuccess)
        {
            _eDevice = ShineWiFi_X; // USB
        }
        delay(1000);
    }
#endif
}

eDevice_t Growatt::GetWiFiStickType()
{
    /**
     * @brief After initialisation the type of the wifi stick is known
     * @returns eDevice_t type of the wifi stick
     */

    return _eDevice;
}

bool Growatt::ReadInputRegisters()
{
    /**
     * @brief Read the input registers from the inverter
     * @returns true if data was read successfully, false otherwise
     */
    uint16_t registerAddress;
    uint8_t res;

    // read each fragment separately
    for (int i = 0; i < _Protocol.InputFragmentCount; i++)
    {
        res = Modbus.readInputRegisters(
            _Protocol.InputReadFragments[i].StartAddress,
            _Protocol.InputReadFragments[i].FragmentSize);
        if (res == Modbus.ku8MBSuccess)
        {
            for (int j = 0; j < _Protocol.InputRegisterCount; j++)
            {
                // make sure the register we try to read is in the fragment
                if (_Protocol.InputRegisters[j].address >= _Protocol.InputReadFragments[i].StartAddress)
                {
                    // when we exceed the fragment size, skip to new fragment
                    if (_Protocol.InputRegisters[j].address >= _Protocol.InputReadFragments[i].StartAddress + _Protocol.InputReadFragments[i].FragmentSize)
                        break;
                    // let's say the register address is 1013 and read window is 1000-1050
                    // that means the response in the buffer is on position 1013 - 1000 = 13
                    registerAddress = _Protocol.InputRegisters[j].address - _Protocol.InputReadFragments[i].StartAddress;
                    if (_Protocol.InputRegisters[j].size == SIZE_16BIT)
                    {
                        _Protocol.InputRegisters[j].value = Modbus.getResponseBuffer(registerAddress);
                    }
                    else
                    {
                        _Protocol.InputRegisters[j].value = (Modbus.getResponseBuffer(registerAddress) << 16) + Modbus.getResponseBuffer(registerAddress + 1);
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool Growatt::ReadHoldingRegisters()
{
    /**
     * @brief Read the holding registers from the inverter
     * @returns true if data was read successfully, false otherwise
     */
    uint16_t registerAddress;
    uint8_t res;

    // read each fragment separately
    for (int i = 0; i < _Protocol.HoldingFragmentCount; i++)
    {
        res = Modbus.readHoldingRegisters(
            _Protocol.HoldingReadFragments[i].StartAddress,
            _Protocol.HoldingReadFragments[i].FragmentSize);
        if (res == Modbus.ku8MBSuccess)
        {
            for (int j = 0; j < _Protocol.HoldingRegisterCount; j++)
            {
                if (_Protocol.HoldingRegisters[j].address >= _Protocol.HoldingReadFragments[i].StartAddress)
                {
                    if (_Protocol.HoldingRegisters[j].address >= _Protocol.HoldingReadFragments[i].StartAddress + _Protocol.HoldingReadFragments[i].FragmentSize)
                        break;
                    registerAddress = _Protocol.HoldingRegisters[j].address - _Protocol.HoldingReadFragments[i].StartAddress;
                    if (_Protocol.HoldingRegisters[j].size == SIZE_16BIT)
                    {
                        _Protocol.HoldingRegisters[j].value = Modbus.getResponseBuffer(registerAddress);
                    }
                    else
                    {
                        _Protocol.HoldingRegisters[j].value = (Modbus.getResponseBuffer(registerAddress) << 16) + Modbus.getResponseBuffer(registerAddress + 1);
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool Growatt::ReadData()
{
    /**
     * @brief Reads the data from the inverter and updates the internal data structures
     * @returns true if data was read successfully, false otherwise
     */

    _PacketCnt++;
    _GotData = ReadInputRegisters() && ReadHoldingRegisters();
    return _GotData;
}

sGrowattModbusReg_t Growatt::GetInputRegister(uint16_t reg)
{
    /**
     * @brief get the internal representation of the input register
     * @param reg the register to get
     * @returns the register value
     */
    if (_GotData == false)
    {
        ReadData();
    }
    return _Protocol.InputRegisters[reg];
}

sGrowattModbusReg_t Growatt::GetHoldingRegister(uint16_t reg)
{
    /**
     * @brief get the internal representation of the holding register
     * @param reg the register to get
     * @returns the register value
     */
    if (_GotData == false)
    {
        ReadData();
    }
    return _Protocol.HoldingRegisters[reg];
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint16_t* result)
{
    /**
     * @brief read 16b holding register
     * @param adr address of the register
     * @param result pointer to the result
     * @returns true if successful
     */
    uint8_t res = Modbus.readHoldingRegisters(adr, 1);
    if (res == Modbus.ku8MBSuccess)
    {
        *result = Modbus.getResponseBuffer(0);
        return true;
    }
    return false;
}

bool Growatt::ReadHoldingReg(uint16_t adr, uint32_t* result)
{
    /**
     * @brief read 32b holding register
     * @param adr address of the register
     * @param result pointer to the result
     * @returns true if successful
     */
    uint8_t res = Modbus.readHoldingRegisters(adr, 2);
    if (res == Modbus.ku8MBSuccess)
    {
        *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
        return true;
    }
    return false;
}

bool Growatt::WriteHoldingReg(uint16_t adr, uint16_t value)
{
    /**
     * @brief write 16b holding register
     * @param adr address of the register
     * @param value value to write to the register
     * @returns true if successful
     */
    uint8_t res = Modbus.writeSingleRegister(adr, value);
    if (res == Modbus.ku8MBSuccess)
    {
        return true;
    }
    return false;
}

bool Growatt::ReadInputReg(uint16_t adr, uint16_t* result)
{
    /**
     * @brief read 16b input register
     * @param adr address of the register
     * @param result pointer to the result
     * @returns true if successful
     */
    uint8_t res = Modbus.readInputRegisters(adr, 1);
    if (res == Modbus.ku8MBSuccess)
    {
        *result = Modbus.getResponseBuffer(0);
        return true;
    }
    return false;
}

bool Growatt::ReadInputReg(uint16_t adr, uint32_t* result)
{
    /**
     * @brief read 32b input register
     * @param adr address of the register
     * @param result pointer to the result
     * @returns true if successful
     */
    uint8_t res = Modbus.readInputRegisters(adr, 2);
    if (res == Modbus.ku8MBSuccess)
    {
        *result = (Modbus.getResponseBuffer(0) << 16) + Modbus.getResponseBuffer(1);
        return true;
    }
    return false;
}

double Growatt::_round2(double value)
{
    return (int)(value * 100 + 0.5) / 100.0;
}

void Growatt::CreateJson(char* Buffer, const char* MacAddress)
{
    StaticJsonDocument<2048> doc;

#if SIMULATE_INVERTER != 1
    for (int i = 0; i < _Protocol.InputRegisterCount; i++)
    {
        if (_Protocol.InputRegisters[i].multiplier == (int)_Protocol.InputRegisters[i].multiplier)
        {
            doc[_Protocol.InputRegisters[i].name] = _Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier;
        }
        else
        {
            doc[_Protocol.InputRegisters[i].name] = _round2(_Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier);
        }
    }
    for (int i = 0; i < _Protocol.HoldingRegisterCount; i++)
    {
        if (_Protocol.HoldingRegisters[i].multiplier == (int)_Protocol.HoldingRegisters[i].multiplier)
        {
            doc[_Protocol.HoldingRegisters[i].name] = _Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier;
        }
        else
        {
            doc[_Protocol.HoldingRegisters[i].name] = _round2(_Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier);
        }
    }
#else
    #warning simulating the inverter
    doc["Status"] = 1;
    doc["DcPower"] = 230;
    doc["DcVoltage"] = 70.5;
    doc["DcInputCurrent"] = 8.5;
    doc["AcFreq"] = 50.00;
    doc["AcVoltage"] = 230.0;
    doc["AcPower"] = 0.00;
    doc["EnergyToday"] = 0.3;
    doc["EnergyTotal"] = 49.1;
    doc["OperatingTime"] = 123456;
    doc["Temperature"] = 21.12;
    doc["AccumulatedEnergy"] = 320;
    doc["EnergyToday"] = 0.3;
    doc["EnergyToday"] = 0.3;
#endif // SIMULATE_INVERTER
    doc["Mac"] = MacAddress;
    doc["Cnt"] = _PacketCnt;
    serializeJson(doc, Buffer, MQTT_MAX_PACKET_SIZE);
}

void Growatt::CreateUIJson(char* Buffer)
{
    StaticJsonDocument<2048> doc;
    const char* unitStr[] = {"", "W", "kWh", "V", "A", "s", "%", "Hz", "C"};

#if SIMULATE_INVERTER != 1
    for (int i = 0; i < _Protocol.InputRegisterCount; i++)
    {
        if (_Protocol.InputRegisters[i].frontend == true || _Protocol.InputRegisters[i].plot == true)
        {
            JsonArray arr = doc.createNestedArray(_Protocol.InputRegisters[i].name);

            // value
            if (_Protocol.InputRegisters[i].multiplier == (int)_Protocol.InputRegisters[i].multiplier)
            {
                arr.add(_Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier);
            }
            else
            {
                arr.add(_round2(_Protocol.InputRegisters[i].value * _Protocol.InputRegisters[i].multiplier));
            }
            arr.add(unitStr[_Protocol.InputRegisters[i].unit]); // unit
            arr.add(_Protocol.InputRegisters[i].plot);          // should be plotted
        }
    }
    for (int i = 0; i < _Protocol.HoldingRegisterCount; i++)
    {
        if (_Protocol.HoldingRegisters[i].frontend == true || _Protocol.HoldingRegisters[i].plot == true)
        {
            JsonArray arr = doc.createNestedArray(_Protocol.HoldingRegisters[i].name);

            // value
            if (_Protocol.HoldingRegisters[i].multiplier == (int)_Protocol.HoldingRegisters[i].multiplier)
            {
                arr.add(_Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier);
            }
            else
            {
                arr.add(_round2(_Protocol.HoldingRegisters[i].value * _Protocol.HoldingRegisters[i].multiplier));
            }
            arr.add(unitStr[_Protocol.HoldingRegisters[i].unit]);
            arr.add(_Protocol.HoldingRegisters[i].plot); // should be plotted
        }
    }
#else
    #warning simulating the inverter
    JsonArray arr = doc.createNestedArray("Status");
    arr.add(1);
    arr.add("");
    arr.add(false);
    arr = doc.createNestedArray("DcPower");
    arr.add(230);
    arr.add("W");
    arr.add(true);
    arr = doc.createNestedArray("DcVoltage");
    arr.add(70.5);
    arr.add("V");
    arr.add(false);
    arr = doc.createNestedArray("DcInputCurrent");
    arr.add(8.5);
    arr.add("A");
    arr.add(false);
    arr = doc.createNestedArray("AcFreq");
    arr.add(50);
    arr.add("Hz");
    arr.add(false);
    arr = doc.createNestedArray("AcVoltage");
    arr.add(230);
    arr.add("V");
    arr.add(false);
    arr = doc.createNestedArray("AcPower");
    arr.add(0.00);
    arr.add("W");
    arr.add(false);
    arr = doc.createNestedArray("EnergyToday");
    arr.add(0.3);
    arr.add("kWh");
    arr.add(false);
    arr = doc.createNestedArray("EnergyTotal");
    arr.add(49.1);
    arr.add("kWh");
    arr.add(false);
    arr = doc.createNestedArray("OperatingTime");
    arr.add(123456);
    arr.add("s");
    arr.add(false);
    arr = doc.createNestedArray("Temperature");
    arr.add(21.12);
    arr.add("C");
    arr.add(false);
    arr = doc.createNestedArray("AccumulatedEnergy");
    arr.add(320);
    arr.add("kWh");
    arr.add(false);
    arr = doc.createNestedArray("EnergyToday");
    arr.add(0.3);
    arr.add("kWh");
    arr.add(false);
#endif // SIMULATE_INVERTER

    serializeJson(doc, Buffer, 4096);
}
