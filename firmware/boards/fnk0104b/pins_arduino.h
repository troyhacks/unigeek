//
// Freenove FNK0104B — ESP32-S3-WROOM-1 (N16R8), 16MB flash, 8MB Quad (QSPI) PSRAM.
// 2.8" 240x320 IPS, ILI9341 4-wire SPI, FT6336U I2C touch, ES8311 codec + FM8002E amp.
// TP4054 LiPo charger (1S), WS2812B RGB LED, single user button.
// SD card wired for 4-bit SDMMC (not SPI — unigeek currently has no SDMMC driver).
//
//   Display — 4-wire SPI to ILI9341 (HSPI on the ESP32-S3):
//     MOSI=11, SCLK=12, CS=10, DC=46, RST=-1 (board-asserted), BL=45 (active HIGH).
//   Touch — I2C to FT6336U:
//     SDA=16, SCL=15, addr=0x38, INT=17, RST=18.
//   I2S audio — to ES8311 codec + FM8002E amp:
//     MCK=4, BCK=5, DIN=6, DOUT=8, WS=7. Amp enable on GPIO 1 (active LOW).
//     Codec on shared I2C bus at 0x18.
//   SD card — 4-bit SDMMC (NOT SPI):
//     CMD=40, CLK=38, D0=39, D1=41, D2=48, D3=47.
//     NOTE: unigeek's initStorage() only knows SPI SD (looks for SD_CS macro).
//     SDMMC support would be a follow-up. For now the board falls back to LittleFS.
//   RGB LED — WS2812B on GPIO 42 (RMT channel 0).
//     NOTE: WS2812 driver not currently in the codebase. Pin defined for reference;
//     uncomment and add Freenove_WS2812_Lib_for_ESP32 (or similar) to lib_deps to enable.
//   Battery — TP4054 charger, voltage divider on GPIO 9 (1/2 ratio).
//   User button — GPIO 0, active LOW, INPUT_PULLUP (strapping pin — release before reset).
//
// Pin provenance: Freenove wiki (docs.freenove.com/projects/fnk0104), Freenove
// GitHub repo (Freenove/Freenove_ESP32_S3_Display, branch main, June 2026).
//

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// ─── I2C (shared bus: FT6336U touch + ES8311 codec) ─────────
#define INTERNAL_SDA  16
#define INTERNAL_SCL  15

static const uint8_t SDA = INTERNAL_SDA;
static const uint8_t SCL = INTERNAL_SCL;

// ─── Touch (FT6336U on shared I2C, addr 0x38) ──────────────
#define TOUCH_SDA       INTERNAL_SDA
#define TOUCH_SCL       INTERNAL_SCL
#define TOUCH_I2C_ADDR  0x38
#define TOUCH_INT       17
#define TOUCH_RST       18

// ─── Codec (ES8311 on shared I2C, addr 0x18) ───────────────
#define CODEC_I2C_ADDR  0x18

// ─── SPI Bus (used by TFT_eSPI display; no SPI SD on this board) ───
//   These constants mirror m5_cores3's pattern. SS is required as a
//   default-arg placeholder for Arduino's SD.h, which is unconditionally
//   included by unigeek's core/StorageSD.h. The FNK0104B has no SPI SD
//   (only 4-bit SDMMC, which initStorage() does not handle), so SPI_SS_PIN
//   is -1. initStorage() falls back to LittleFS.
//
//   Note: literal pin numbers here (not references to TFT_MOSI / TFT_SCLK,
//   which are defined later in the file). m5_cores3 uses literals too.
#define SPI_SS_PIN    -1
#define SPI_SCK_PIN   12
#define SPI_MISO_PIN  -1
#define SPI_MOSI_PIN  11

static const uint8_t SS   = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t MISO = SPI_MISO_PIN;
static const uint8_t SCK  = SPI_SCK_PIN;

// ─── I2S audio (ES8311 codec + FM8002E amp) ───────────────
#define I2S_MCK      4
#define I2S_BCK      5
#define I2S_DIN      6
#define I2S_DOUT     8
#define I2S_WS       7
#define AMP_ENABLE   1    // FM8002E amp enable, active LOW

// Speaker pins for SpeakerI2S (firmware/src/core/SpeakerI2S.h) — these
// names are required by the shared I2S driver. MCLK is unused (the
// ES8311 takes MCLK from its own BCK via internal PLL), so SpeakerI2S
// leaves mck_io_num = I2S_PIN_NO_CHANGE.
#define SPK_BCLK      I2S_BCK
#define SPK_WCLK      I2S_WS
#define SPK_DOUT      I2S_DOUT
#define SPK_I2S_PORT  I2S_NUM_0

// ─── SD card (4-bit SDMMC — NOT currently supported by unigeek) ─
//   Defining pins here for documentation; unigeek's initStorage() only
//   initialises SPI SD via the SD_CS macro. With no SD_CS defined,
//   the board falls back to LittleFS. SDMMC support is a follow-up.
#define SD_CMD       40
#define SD_CLK       38
#define SD_D0        39
#define SD_D1        41
#define SD_D2        48
#define SD_D3        47
// #define SD_CS  <undefined — would activate SPI SD fallback if set>

// ─── WS2812B RGB LED (driver not yet in codebase) ──────────
//   Pin defined for future use. To enable, add a WS2812 lib (e.g.
//   Freenove_WS2812_Lib_for_ESP32) to lib_deps and a small driver in
//   firmware/boards/fnk0104b/lib/.
#define RGB_PIN      42   // WS2812 DIN, RMT channel 0

// ─── Battery (TP4054 + 1/2 voltage divider on ADC1_CH8) ─────
#define BAT_ADC      9

// ─── User button (active LOW, INPUT_PULLUP) ────────────────
#define BTN_PIN      0

// ─── Display geometry (portrait-native, landscape via rotation) ──
#define TFT_WIDTH   240
#define TFT_HEIGHT  320

// ─── TFT_eSPI config (inline, mirrors m5_cores3 pattern) ───
#define DISABLE_ALL_LIBRARY_WARNINGS 1
#define USER_SETUP_LOADED 1

#define ILI9341_DRIVER
#define TFT_INVERSION_ON                  // FNK0104B panel needs inversion
#define TFT_MOSI    11
#define TFT_SCLK    12
#define TFT_CS      10
#define TFT_DC      46
#define TFT_RST     -1
#define TFT_BL      45
#define TFT_MISO    -1
#define TFT_BACKLIGHT_ON HIGH
#define TOUCH_CS    -1
#define SMOOTH_FONT
#define USE_HSPI_PORT
#define TFT_DEFAULT_ORIENTATION 0          // portrait (240×320) — matches FT6336U's raw touch axes
#define SPI_FREQUENCY        40000000
#define SPI_READ_FREQUENCY   20000000

// ─── Firmware Feature Flags ───────────────────────────────
//   BOARD_HAS_PSRAM is provided via the board JSON's extra_flags
//   (see firmware/boards/_devices/fnk0104b.json) — don't redefine.

#define DEVICE_HAS_USB_HID         // ESP32-S3 native USB HID
#define DEVICE_HAS_WEBAUTHN        // FIDO2 / WebAuthn USB security key
#define DEVICE_HAS_TOUCH_NAV       // touch-only navigation (no physical buttons)
#define DEVICE_HAS_SCREEN_ORIENT   // display rotates 180° for right-hand mode
#define DEVICE_HAS_SOUND           // ES8311 codec + FM8002E amp, I2S out
#define DEVICE_HAS_VOLUME_CONTROL  // I2S amp + ES8311 support setVolume() — shows Volume slider in Settings
// #define DEVICE_HAS_RGB_LED        // WS2812 on GPIO 42 — driver TBD

#endif // Pins_Arduino_h
