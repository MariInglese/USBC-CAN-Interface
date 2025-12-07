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
  canMsg.can_id = 0x123; 
  canMsg.can_dlc = 8; 
  canMsg.data[0] = 0xAA; 
  canMsg.data[1] = 0xBB; 
  canMsg.data[2] = 0xCC; 
  canMsg.data[3] = 0xDD; 
  canMsg.data[4] = 0xEE; 
  canMsg.data[5] = 0xFF; 
  canMsg.data[6] = 0x01; 
  canMsg.data[7] = 0x02; 

  byte status = mcp2515.sendMessage(&canMsg); 

  if(status == MCP2515::ERROR_OK){
    Serial.println("Can message sent successfully"); 
  }
  else{
    Serial.print("Error sending CAN message "); 
    Serial.println(status); 
  }

  // Send every second 
  delay(1000); 
}
