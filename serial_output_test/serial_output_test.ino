// new transmit script
#include <SPI.h>
#include <mcp2515.h>
#include <Adafruit_NeoPixel.h>

#define CAN_CS   5
#define CAN_MOSI 11
#define CAN_MISO 13
#define CAN_SCK  12

#define RGB_PIN 48
#define NUM_PIXELS 1

Adafruit_NeoPixel rgb(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);
MCP2515 mcp2515(CAN_CS);
struct can_frame canMsg;

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

void setup() {
  rgb.begin();
  rgb.setBrightness(20);

  setColor(255, 255, 0);   // 🟨 starting

  SPI.begin(CAN_SCK, CAN_MISO, CAN_MOSI, CAN_CS);

  mcp2515.reset();

  if (mcp2515.setBitrate(CAN_125KBPS, MCP_16MHZ) != MCP2515::ERROR_OK) {
    setColor(255, 0, 0);   // 🟥 bitrate fail
    while (1);
  }

  mcp2515.setNormalMode();

  setColor(0, 255, 0);     // 🟩 init OK
}

void loop() {
  canMsg.can_id  = 0x36;
  canMsg.can_dlc = 1;
  canMsg.data[0] = 0xAA;

  if (mcp2515.sendMessage(&canMsg) == MCP2515::ERROR_OK) {
    setColor(0, 0, 255);   // 🔵 send OK
  } else {
    setColor(255, 0, 0);   // 🟥 send fail
  }

  delay(1000);
}
