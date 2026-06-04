//
// Freenove FNK0104B — touch navigation via FT6336U on shared I2C (Wire, SDA=16, SCL=15).
// Pattern mirrors m5_cores3; the FT6336U reports raw panel coords. With
// TFT_DEFAULT_ORIENTATION=0 the panel is in its native portrait (240x320),
// and we apply the X-flip / Y-mapping ourselves to match the touch zones.
//

#include "Navigation.h"
#include "pins_arduino.h"
#include "core/Device.h"
#include <Arduino.h>
#include <Wire.h>

// SCREEN_W/H are the *visible* screen dimensions. Display is portrait
// (TFT_DEFAULT_ORIENTATION=0, 240 wide × 320 tall), matching the
// FT6336U's raw touch axes (rawX 0..239 left→right, rawY 0..319 top→bottom).
static constexpr int16_t SCREEN_W = TFT_WIDTH;    // 240
static constexpr int16_t SCREEN_H = TFT_HEIGHT;   // 320
static constexpr int16_t BACK_END = SCREEN_W / 4;   //  60 — left 1/4 = BACK
static constexpr int16_t ZONE_H   = SCREEN_H / 3;   // 107 — right 3/4 split top-to-bottom

// Consecutive no-touch polls required to confirm a release (~60ms at 20ms poll rate)
static constexpr uint8_t NO_TOUCH_THRESHOLD = 3;

static bool _gSwapXY    = false;
static bool _gRightHand = false;

void NavigationImpl::setRightHand(bool v)   { _gRightHand = v; }
void NavigationImpl::setTouchSwapXY(bool v) { _gSwapXY    = v; }

void NavigationImpl::begin() {
  // Touch reset: hold RST low for ≥10 ms, then high. FT6336U datasheet timing.
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(15);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);

  touch.begin(Wire, TOUCH_I2C_ADDR);
  pinMode(TOUCH_INT, INPUT);
}

void NavigationImpl::update() {
  static uint32_t lastPoll = 0;
  uint32_t now = millis();

  if (now - lastPoll < 20) {
    updateState(_curDir);
    return;
  }
  lastPoll = now;

  int16_t rawX, rawY;
  if (!touch.read(rawX, rawY)) {
    if (++_noTouchCnt < NO_TOUCH_THRESHOLD) {
      updateState(_curDir);
      return;
    }
    _curDir = DIR_NONE;
    updateState(DIR_NONE);
    return;
  }

  _noTouchCnt = 0;

  // FT6336U on FNK0104B reports in landscape coords (same as m5_cores3):
  //   rawX: 0..319 (left → right),  rawY: 0..239 (top → bottom).
  // Display runs at TFT_DEFAULT_ORIENTATION=1 (landscape 320×240), so the
  // raw touch axes match the visible screen axes — use as-is.
  int16_t sx = rawX;
  int16_t sy = rawY;
  if (sx < 0) sx = 0; if (sx >= SCREEN_W) sx = SCREEN_W - 1;
  if (sy < 0) sy = 0; if (sy >= SCREEN_H) sy = SCREEN_H - 1;

  if (_gRightHand) {
    sx = (SCREEN_W - 1) - sx;
    sy = (SCREEN_H - 1) - sy;
  }

  if (_gSwapXY) {
    int16_t tmp = sx; sx = sy; sy = tmp;
  }

  Direction dir;
  if (sx < BACK_END) {
    dir = DIR_BACK;
  } else {
    if      (sy < ZONE_H)       dir = DIR_UP;
    else if (sy < ZONE_H * 2)   dir = DIR_PRESS;
    else                        dir = DIR_DOWN;
  }

  _lastTouchX = sx;
  _lastTouchY = sy;
  _curDir = dir;
  updateState(_curDir);
}

// ─── Touch-only overlay: at rest nothing is drawn. When a zone is held a
// single 2 px bar lights up on the matching screen edge; on release (or
// zone change) it is cleared back to black. drawOverlay() only emits
// SPI traffic on state transitions, so there is no per-frame flicker
// and no invalidate coupling with chrome refresh.
void NavigationImpl::_paintZone(Direction d, bool lit) {
  static constexpr uint16_t LIT_RED   = 0xF800;
  static constexpr uint16_t LIT_GREEN = 0x07E0;
  static constexpr uint16_t LIT_BLUE  = 0x001F;

  auto& lcd = Uni.Lcd;
  Sprite bar(&lcd);

  switch (d) {
    case DIR_BACK:
      bar.createSprite(2, SCREEN_H);
      bar.fillSprite(lit ? LIT_RED : TFT_BLACK);
      bar.pushSprite(0, 0);
      break;
    case DIR_UP:
      bar.createSprite(2, ZONE_H - 1);
      bar.fillSprite(lit ? LIT_GREEN : TFT_BLACK);
      bar.pushSprite(SCREEN_W - 2, 0);
      break;
    case DIR_PRESS:
      bar.createSprite(2, ZONE_H - 1);
      bar.fillSprite(lit ? LIT_BLUE : TFT_BLACK);
      bar.pushSprite(SCREEN_W - 2, ZONE_H);
      break;
    case DIR_DOWN:
      bar.createSprite(2, SCREEN_H - ZONE_H * 2);
      bar.fillSprite(lit ? LIT_GREEN : TFT_BLACK);
      bar.pushSprite(SCREEN_W - 2, ZONE_H * 2);
      break;
    default:
      return;
  }
  bar.deleteSprite();
}

void NavigationImpl::_doDrawOverlay() {
  if (_curDir == _lastDir) return;
  if (_lastDir != DIR_NONE) _paintZone(_lastDir, false);
  if (_curDir  != DIR_NONE) _paintZone(_curDir,  true);
  _lastDir = _curDir;
}
