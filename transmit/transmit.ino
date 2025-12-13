#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)

#define CAN_CS   5
#define CAN_MOSI 11
#define CAN_MISO 13
#define CAN_SCK  12

MCP2515 mcp2515(CAN_CS);
struct can_frame canMsg;

// Serial command buffer
String serialBuffer = "";

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(CAN_SCK, CAN_MISO, CAN_MOSI, CAN_CS);

  mcp2515.reset();

  // MOST MCP2515 MODULES USE 16 MHz
  mcp2515.setBitrate(CAN_125KBPS, MCP_16MHZ);

  mcp2515.setNormalMode();

  uint8_t status = mcp2515.getStatus();
  Serial.print("READY|STATUS:");
  Serial.println(status, HEX);
}

void loop() {
  // Check for incoming serial data
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    
    if (inChar == '\n') {
      // Process the complete command
      processSerialCommand(serialBuffer);
      serialBuffer = "";
    } else if (inChar != '\r') {
      serialBuffer += inChar;
    }
  }
}

void processSerialCommand(String command) {
  // Expected format: "SEND|ID:0x123|DLC:8|DATA:FF,FF,FF,FF,00,00,00,00"
  
  if (!command.startsWith("SEND|")) {
    Serial.println("ERROR|Invalid command format");
    return;
  }

  // Parse the command
  int id = 0;
  int dlc = 0;
  uint8_t data[8] = {0};

  // Extract ID
  int idStart = command.indexOf("ID:") + 3;
  int idEnd = command.indexOf("|", idStart);
  String idStr = command.substring(idStart, idEnd);
  id = strtol(idStr.c_str(), NULL, 16);

  // Extract DLC
  int dlcStart = command.indexOf("DLC:") + 4;
  int dlcEnd = command.indexOf("|", dlcStart);
  String dlcStr = command.substring(dlcStart, dlcEnd);
  dlc = dlcStr.toInt();
  if (dlc > 8) dlc = 8;

  // Extract DATA
  int dataStart = command.indexOf("DATA:") + 5;
  String dataStr = command.substring(dataStart);
  
  // Parse hex values separated by commas
  int dataIndex = 0;
  int lastPos = 0;
  for (int i = 0; i < dataStr.length() && dataIndex < dlc; i++) {
    if (dataStr[i] == ',' || i == dataStr.length() - 1) {
      int endPos = (i == dataStr.length() - 1) ? i + 1 : i;
      String hexStr = dataStr.substring(lastPos, endPos);
      data[dataIndex] = strtol(hexStr.c_str(), NULL, 16);
      dataIndex++;
      lastPos = i + 1;
    }
  }

  // Create and send CAN message
  canMsg.can_id = id;
  canMsg.can_dlc = dlc;
  for (int i = 0; i < dlc; i++) {
    canMsg.data[i] = data[i];
  }

  if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print("TX|ID:0x");
    Serial.print(id, HEX);
    Serial.print("|DLC:");
    Serial.print(dlc);
    Serial.print("|DATA:");
    for (int i = 0; i < dlc; i++) {
      Serial.print(data[i], HEX);
      if (i < dlc - 1) Serial.print(",");
    }
    Serial.println();
  } else {
    Serial.println("ERROR|TX Failed");
  }
}
