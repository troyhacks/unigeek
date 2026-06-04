//
// Freenove FNK0104B — battery and power management.
// TP4054 linear charger + 1/2 voltage divider on GPIO 9 (ADC1_CH8).
// powerOff() uses deep sleep; wake on touch INT (GPIO 17, active LOW).
//
// Battery percentage is computed by mapping ADC millivolts to 0..100% across
// a typical 1S LiPo range (3.0 V empty to 4.2 V full). The TP4054 has no fuel
// gauge, so this is a rough estimate.
//

#pragma once

#include "core/IPower.h"
#include "pins_arduino.h"
#include <Arduino.h>
#include <esp_sleep.h>

class PowerImpl : public IPower
{
public:
  void begin() override {
    analogReadResolution(12);                 // ESP32-S3 default; explicit for clarity
    analogSetPinAttenuation(BAT_ADC, ADC_11db);  // ~0–3.3 V full scale
  }

  uint8_t getBatteryPercentage() override {
    // Two reads averaged; the ADC on ESP32-S3 is noisy at high impedance.
    uint32_t mv = (analogReadMilliVolts(BAT_ADC) + analogReadMilliVolts(BAT_ADC)) / 2;
    // 1/2 divider means the divider output is half the battery voltage.
    uint32_t battery_mv = mv * 2;

    if (battery_mv <= 3000) return 0;
    if (battery_mv >= 4200) return 100;
    return (uint8_t)((battery_mv - 3000) * 100 / (4200 - 3000));
  }

  // TP4054 has a CHRG pin (active LOW) but it's not broken out to a GPIO on
  // this board. Return false (not charging) as a conservative default.
  bool isCharging() override { return false; }

  void powerOff() override {
    // Wake on touch (FT6336U INT, active LOW).
    esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_INT, LOW);
    esp_deep_sleep_start();
  }
};
