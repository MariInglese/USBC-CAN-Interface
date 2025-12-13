#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)

#define LED_PIN 7

struct can_frame canMsg;
MCP2515 mcp2515(10);                 // SPI CS Pin 10


void setup()
{
  Serial.begin(115200);  // Changed to match transmit baud rate
  pinMode(LED_PIN, OUTPUT);
  SPI.begin();
  delay(3000);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("READY|Receiver initialized");
}


void loop()
{
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    // Toggle LED on message receive
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    
    // Format and send message to serial
    Serial.print("RX|ID:0x");
    Serial.print(canMsg.can_id, HEX);
    Serial.print("|DLC:");
    Serial.print(canMsg.can_dlc);
    Serial.print("|DATA:");
    
    for (int i = 0; i < canMsg.can_dlc; i++) {
      Serial.print(canMsg.data[i], HEX);
      if (i < canMsg.can_dlc - 1) Serial.print(",");
    }
    Serial.println();
    
    delay(100);
  }
}
