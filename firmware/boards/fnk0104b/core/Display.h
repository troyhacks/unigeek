//
// Freenove FNK0104B — DisplayImpl with LEDC PWM backlight on GPIO 45.
// Display is a stock ILI9341 on 4-wire HSPI; all panel config lives in pins_arduino.h.
//

#pragma once

#include "core/IDisplay.h"
#include "pins_arduino.h"

class DisplayImpl : public IDisplay
{
public:
  void setBrightness(uint8_t pct) override {
    if (pct > 100) pct = 100;
    static bool _ready = false;
    if (!_ready) {
      ledcSetup(0, 5000, 8);
      ledcAttachPin(TFT_BL, 0);
      _ready = true;
    }
    ledcWrite(0, (uint8_t)((uint32_t)pct * 255 / 100));
  }
};
