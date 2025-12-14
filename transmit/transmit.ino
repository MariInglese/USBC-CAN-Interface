#include <SPI.h>
#include <mcp2515.h>

// ================= SPI PINS =================
#define SPI_SCK   12
#define SPI_MOSI  11
#define SPI_MISO  13

// ================= MCP2515 =================
#define CAN_CS_PIN 10
#define CAN_INT_PIN 9

struct can_frame canMsg;
MCP2515 mcp2515(CAN_CS_PIN);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // SPI init
  SPI.begin();

  // MCP2515 init
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  Serial.println("READY|CAN Transmitter initialized");
}

// ================= PARSE HEX BYTE =================
byte parseHexByte(String hexStr) {
  return (byte)strtol(hexStr.c_str(), NULL, 16);
}

// ================= LOOP =================
void loop() {
  // Check for incoming serial commands from UI
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    Serial.print("DEBUG|Received command: ");
    Serial.println(command);
    
    if (command.startsWith("SEND|")) {
      // Parse command format: SEND|ID:0x{id}|DLC:{dlc}|DATA:{data}
      // Example: SEND|ID:0x123|DLC:8|DATA:FF,FF,FF,FF,00,00,00,00
      
      int idStart = command.indexOf("ID:") + 3;
      int idEnd = command.indexOf("|", idStart);
      String idStr = command.substring(idStart, idEnd);
      
      int dlcStart = command.indexOf("DLC:") + 4;
      int dlcEnd = command.indexOf("|", dlcStart);
      String dlcStr = command.substring(dlcStart, dlcEnd);
      
      int dataStart = command.indexOf("DATA:") + 5;
      String dataStr = command.substring(dataStart);
      
      Serial.print("DEBUG|Parsed ID: ");
      Serial.println(idStr);
      Serial.print("DEBUG|Parsed DLC: ");
      Serial.println(dlcStr);
      Serial.print("DEBUG|Parsed DATA: ");
      Serial.println(dataStr);
      
      // Parse CAN ID (handle 0x prefix)
      canMsg.can_id = strtol(idStr.c_str(), NULL, 16);
      
      // Parse DLC
      canMsg.can_dlc = (byte)atoi(dlcStr.c_str());
      
      if (canMsg.can_dlc > 8) {
        canMsg.can_dlc = 8;
      }
      
      Serial.print("DEBUG|CAN ID (hex): ");
      Serial.print(canMsg.can_id, HEX);
      Serial.print(" | DLC: ");
      Serial.println(canMsg.can_dlc);
      
      // Parse data bytes
      int byteIndex = 0;
      int lastComma = -1;
      for (int i = 0; i <= dataStr.length() && byteIndex < canMsg.can_dlc; i++) {
        if (dataStr[i] == ',' || i == dataStr.length()) {
          String byteStr = dataStr.substring(lastComma + 1, i);
          byteStr.trim();
          if (byteStr.length() > 0) {
            canMsg.data[byteIndex] = parseHexByte(byteStr);
            Serial.print("DEBUG|Parsed byte ");
            Serial.print(byteIndex);
            Serial.print(": ");
            Serial.println(canMsg.data[byteIndex], HEX);
            byteIndex++;
          }
          lastComma = i;
        }
      }
      
      Serial.println("DEBUG|Attempting to send CAN message...");
      
      // Send the CAN message
      if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
        Serial.print("TX|ID: 0x");
        Serial.print(canMsg.can_id, HEX);
        Serial.print(" | DLC: ");
        Serial.print(canMsg.can_dlc);
        Serial.print(" | Data: ");
        for (int i = 0; i < canMsg.can_dlc; i++) {
          if (canMsg.data[i] < 0x10) Serial.print("0");
          Serial.print(canMsg.data[i], HEX);
          if (i < canMsg.can_dlc - 1) Serial.print(",");
        }
        Serial.println();
      } else {
        Serial.println("ERROR|Failed to send CAN message");
      }
    } else {
      Serial.print("ERROR|Unknown command format: ");
      Serial.println(command);
    }
  }
  
  delay(10);
}
