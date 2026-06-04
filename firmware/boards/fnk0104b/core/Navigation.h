//
// Freenove FNK0104B — touch-based navigation (no physical buttons).
// FT6336U on shared I2C (Wire, SDA=16, SCL=15, addr=0x38).
//
// Touch zones (landscape 320x240 — TFT_eSPI setRotation(1)):
//   Left 1/4  (x < 80):                  BACK
//   Right 3/4 (x >= 80), top 1/3:        UP
//   Right 3/4 (x >= 80), middle 1/3:     SELECT (PRESS)
//   Right 3/4 (x >= 80), bottom 1/3:     DOWN
//
// _doDrawOverlay() paints a 2 px edge bar for the active zone; clears it on release.
//

#pragma once

#include "core/INavigation.h"
#include "../lib/TouchFT6336U.h"

class NavigationImpl : public INavigation
{
public:
  void begin()                override;
  void update()               override;
  void setRightHand(bool v)   override;
  void setTouchSwapXY(bool v) override;

  TouchFT6336U touch;

protected:
  void _doDrawOverlay() override;

private:
  Direction _curDir     = DIR_NONE;
  Direction _lastDir    = DIR_NONE;
  uint8_t   _noTouchCnt = 0;

  void _paintZone(Direction d, bool lit);
};
