// new
#include "ModbusTCP.h"

ModbusTCP::ModbusTCP(uint16_t serverPort)
  : port(serverPort), enabled(false) {
  server = nullptr;
}

ModbusTCP::~ModbusTCP() {
  stop();
}

void ModbusTCP::begin() {
  if (server == nullptr) {
    server = new WiFiServer(port);
  }
  server->begin();
  enabled = true;
  Serial.print(F("Modbus TCP server started on port "));
  Serial.println(port);
}

void ModbusTCP::stop() {
  if (server != nullptr) {
    server->stop();
    delete server;
    server = nullptr;
  }
  if (client) {
    client.stop();
  }
  enabled = false;
}

void ModbusTCP::loop() {
  if (!enabled || server == nullptr) return;

  // Check for new client
  if (!client || !client.connected()) {
    client = server->available();
    if (!client) return;
  }

  // Check if data available
  if (client.available() >= 12) {  // Minimum Modbus TCP request size
    uint16_t bytesRead = 0;

    // Read MBAP header (7 bytes) + PDU
    while (client.available() && bytesRead < sizeof(requestBuffer)) {
      requestBuffer[bytesRead++] = client.read();
    }

    if (bytesRead >= 12) {
      processRequest();
    }
  }
}

void ModbusTCP::processRequest() {
  // Parse MBAP header
  uint16_t transactionId = (requestBuffer[0] << 8) | requestBuffer[1];
  uint16_t protocolId = (requestBuffer[2] << 8) | requestBuffer[3];
  uint16_t length = (requestBuffer[4] << 8) | requestBuffer[5];
  uint8_t unitId = requestBuffer[6];
  uint8_t functionCode = requestBuffer[7];

  // Validate protocol ID
  if (protocolId != 0) {
    client.stop();
    return;
  }

  // Build response header
  responseBuffer[0] = requestBuffer[0];  // Transaction ID high
  responseBuffer[1] = requestBuffer[1];  // Transaction ID low
  responseBuffer[2] = 0;                 // Protocol ID high
  responseBuffer[3] = 0;                 // Protocol ID low
  responseBuffer[6] = unitId;            // Unit ID
  responseBuffer[7] = functionCode;      // Function code

  uint16_t responseLength = 0;

  switch (functionCode) {
    case FC_READ_HOLDING_REGISTERS:
    case FC_READ_INPUT_REGISTERS:
      {
        uint16_t startAddress = (requestBuffer[8] << 8) | requestBuffer[9];
        uint16_t quantity = (requestBuffer[10] << 8) | requestBuffer[11];

        if (quantity < 1 || quantity > 125) {
          sendException(functionCode, EX_ILLEGAL_DATA_VALUE);
          return;
        }

        uint8_t byteCount = quantity * 2;
        responseBuffer[8] = byteCount;

        bool success = true;
        for (uint16_t i = 0; i < quantity; i++) {
          uint16_t value = 0;
          bool result = false;

          if (functionCode == FC_READ_HOLDING_REGISTERS && readHoldingRegister) {
            result = readHoldingRegister(startAddress + i, &value);
          } else if (functionCode == FC_READ_INPUT_REGISTERS && readInputRegister) {
            result = readInputRegister(startAddress + i, &value);
          }

          if (!result) {
            sendException(functionCode, EX_ILLEGAL_DATA_ADDRESS);
            return;
          }

          responseBuffer[9 + (i * 2)] = value >> 8;
          responseBuffer[10 + (i * 2)] = value & 0xFF;
        }

        responseLength = 9 + byteCount;
        break;
      }

    case FC_WRITE_SINGLE_REGISTER:
      {
        uint16_t address = (requestBuffer[8] << 8) | requestBuffer[9];
        uint16_t value = (requestBuffer[10] << 8) | requestBuffer[11];

        if (writeHoldingRegister && writeHoldingRegister(address, value)) {
          // Echo back the request for write single register
          responseBuffer[8] = requestBuffer[8];
          responseBuffer[9] = requestBuffer[9];
          responseBuffer[10] = requestBuffer[10];
          responseBuffer[11] = requestBuffer[11];
          responseLength = 12;
        } else {
          sendException(functionCode, EX_ILLEGAL_DATA_ADDRESS);
          return;
        }
        break;
      }

    default:
      sendException(functionCode, EX_ILLEGAL_FUNCTION);
      return;
  }

  // Set length field (Unit ID + PDU length)
  uint16_t mbapLength = responseLength - 6;
  responseBuffer[4] = mbapLength >> 8;
  responseBuffer[5] = mbapLength & 0xFF;

  // Send response
  client.write(responseBuffer, responseLength);
  client.flush();
}

void ModbusTCP::sendException(uint8_t functionCode, uint8_t exceptionCode) {
  responseBuffer[7] = functionCode | 0x80;  // Set exception bit
  responseBuffer[8] = exceptionCode;

  // Set length (3 bytes: Unit ID + Function code + Exception code)
  responseBuffer[4] = 0;
  responseBuffer[5] = 3;

  client.write(responseBuffer, 9);
  client.flush();
}

// ============================================================================
// MODIFICATIONS TO MAIN FILE (ShineWiFi-ModBus.txt)
// ============================================================================

// 1. Add near the top with other includes:
/*
#if MODBUS_TCP_SUPPORTED == 1
#include "ModbusTCP.h"
#endif
*/

// 2. Add to Config.h:
/*
#define MODBUS_TCP_SUPPORTED 1
#define MODBUS_TCP_PORT 502
*/

// 3. Add global instance after other globals (around line 60):
/*
#if MODBUS_TCP_SUPPORTED == 1
ModbusTCP modbusTCP(MODBUS_TCP_PORT);
#endif
*/

// 4. Add callback functions before setup():
/*
#if MODBUS_TCP_SUPPORTED == 1
bool modbusReadHoldingRegister(uint16_t address, uint16_t* value) {
  return Inverter.ReadHoldingReg(address, value);
}

bool modbusReadInputRegister(uint16_t address, uint16_t* value) {
  return Inverter.ReadInputReg(address, value);
}

bool modbusWriteHoldingRegister(uint16_t address, uint16_t value) {
  return Inverter.WriteHoldingReg(address, value);
}
#endif
*/

// 5. Add to setup() function, after httpServer.begin():
/*
#if MODBUS_TCP_SUPPORTED == 1
  modbusTCP.readHoldingRegister = modbusReadHoldingRegister;
  modbusTCP.readInputRegister = modbusReadInputRegister;
  modbusTCP.writeHoldingRegister = modbusWriteHoldingRegister;
  modbusTCP.begin();
#endif
*/

// 6. Add to loop() function, after httpServer.handleClient():
/*
#if MODBUS_TCP_SUPPORTED == 1
  if (modbusTCP.isEnabled()) {
    modbusTCP.loop();
  }
#endif
*/

// ============================================================================
// USAGE EXAMPLE
// ============================================================================
/*
Once implemented, you can connect to the ESP using any Modbus TCP client:

Example using pymodbus (Python):
```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient('192.168.1.100', port=502)
client.connect()

# Read 10 input registers starting at address 0
result = client.read_input_registers(0, 10, unit=1)
if not result.isError():
    print(result.registers)

# Read holding registers
result = client.read_holding_registers(0, 10, unit=1)
if not result.isError():
    print(result.registers)

# Write single holding register
client.write_register(0, 100, unit=1)

client.close()
```

Register mapping will match your Growatt inverter's Modbus map.
*/
