#include <mcp2515.h>
#include <SPI.h>

// Create an instance of the MCP2515 class (CS Pin = 10)
MCP2515 mcp2515(10)

void setup() {
  // Initialize the MCP2515 module 
  mcp2515.reset(); 
  mcp2515.setBitrate(CAN_500KBPS); // Bit rate = 500 KBPS
  mcp2515.setNormalMode(); 
}

void loop() {
  // put your main code here, to run repeatedly:

}