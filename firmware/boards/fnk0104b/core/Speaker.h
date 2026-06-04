//
// Freenove FNK0104B — I2S speaker with ES8311 codec init over Wire.
// Mirrors m5_cardputer_adv's SpeakerADV pattern, with two differences:
//   * Uses Wire (not Wire1) — the FNK0104B has only one I2C bus, shared
//     between the FT6336U touch (0x38) and the ES8311 codec (0x18).
//   * Drives AMP_ENABLE LOW before SpeakerI2S::begin() to wake the
//     FM8002E amp; the m5_cores3 amp (AW88298) is I2C-controlled instead.
//
// ES8311 sits at I2C addr 0x18 on the same bus (SDA=16, SCL=15). Wire is
// already started by Device::createInstance() at 400 kHz before this
// begin() runs.
//

#pragma once

#include "core/SpeakerI2S.h"
#include "pins_arduino.h"
#include <Wire.h>

class SpeakerFNK : public SpeakerI2S {
public:
  void begin() override {
    // Wake the FM8002E amp. Active LOW.
    pinMode(AMP_ENABLE, OUTPUT);
    digitalWrite(AMP_ENABLE, LOW);

    SpeakerI2S::begin();
    _initES8311();
  }

private:
  void _initES8311() {
    // Standard ES8311 init sequence (per ES8311 application note).
    // Same register table as m5_cardputer_adv's SpeakerADV — the codec
    // init is chip-specific, not board-specific, and the FNK0104B's
    // ES8311 is a stock part at I2C addr 0x18.
    static constexpr uint8_t regs[][2] = {
      {0x00, 0x80},  // RESET / CSM POWER ON
      {0x01, 0xB5},  // CLOCK_MANAGER / MCLK=BCLK
      {0x02, 0x18},  // CLOCK_MANAGER / MULT_PRE=3
      {0x0D, 0x01},  // SYSTEM / Power up analog circuitry
      {0x12, 0x00},  // SYSTEM / Power-up DAC
      {0x13, 0x10},  // SYSTEM / Enable output to HP drive
      {0x32, 0xBF},  // DAC / Volume ±0 dB (max)
      {0x37, 0x08},  // DAC / Bypass DAC equalizer
    };
    for (const auto& r : regs) {
      Wire.beginTransmission(CODEC_I2C_ADDR);
      Wire.write(r[0]);
      Wire.write(r[1]);
      Wire.endTransmission();
    }
  }
};
