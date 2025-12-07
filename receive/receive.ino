#include <SPI.h>
#include <mcp2515.h>

// Chip select pin for transmit MCP2515
const int SPI_CS_PIN = 10; 

// Create an instance of the MCP2515 library 
MCP2515 mcp2515(SPI_CS_PIN); 

void setup() {
  Serial.begin(115200); 
  SPI.begin(); 

  // Initialize MCP2515
  mcp2515.reset(); 
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); 
  mcp2515.setNormalMode(); 

  Serial.println("MCP2515 Initialized"); 
}

void loop() {
  can_frame canMsg; 
  
  // Check for recieved messages
  if(mcp2515.checkReceive() == MCP2515::ERROR_OK){
    byte status = mcp2515.readMessage(&canMsg); 

    // Print recieved messages
    if(status == MCP2515::ERROR_OK){
      Serial.print("Received CAN message - ID: 0x");
      Serial.print(canMsg.can_id, HEX);
      Serial.print(", DLC: ");
      Serial.print(canMsg.can_dlc);
      Serial.print(", Data: ");

      for (int i = 0; i < canMsg.can_dlc; i++) {
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    else{
      Serial.print("Error reading CAN message: ");
      Serial.println(status);
    }
  }
}


