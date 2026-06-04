//
// Freenove FNK0104B — Device factory.
//   Display:  HSPI 4-wire (MOSI=11, SCLK=12, CS=10, DC=46) — ILI9341 240x320
//   Touch:    FT6336U on shared I2C (SDA=16, SCL=15, addr=0x38, INT=17, RST=18)
//   Battery:  TP4054 + 1/2 divider on GPIO 9
//   Button:   GPIO 0, active LOW, INPUT_PULLUP
//   SD card:  SDMMC 4-bit (CMD=40, CLK=38, D0=39, D1=41, D2=48, D3=47) —
//             unigeek has no SDMMC driver yet, so no SD_CS is defined and
//             initStorage() falls back to LittleFS.
//

#include "core/Device.h"
#include "Display.h"
#include "Navigation.h"
#include "Power.h"
#include "Speaker.h"
#include <Wire.h>

static DisplayImpl    display;
static NavigationImpl navigation;
static PowerImpl      power;
static SpeakerFNK     speaker;

void Device::applyNavMode() {}
void Device::boardHook()  {}

Device* Device::createInstance() {
  // Enable backlight before display init so the screen illuminates immediately.
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Bring up the shared I2C bus (FT6336U touch + ES8311 codec share it).
  // 400 kHz is recommended for ES8311 codec register access.
  Wire.begin(INTERNAL_SDA, INTERNAL_SCL, 400000UL);

  auto* dev = new Device(display, power, &navigation, nullptr, nullptr, &speaker);
  dev->ExI2C = &Wire;
  return dev;
}
