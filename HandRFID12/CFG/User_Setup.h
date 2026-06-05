// ============================================================
//  TFT_eSPI User_Setup.h for CYD boards
//  Supports: ESP32-2432S024R/C (2.4") / ESP32-2432S028R (2.8")
//
//  IMPORTANT: Copy this file to your TFT_eSPI library folder:
//  Arduino/libraries/TFT_eSPI/User_Setup.h
//  (overwrite the existing one)
//
//  *** DISPLAY SELECTION: must match config.h! ***
// ============================================================

// ============================================================
//  DISPLAY SELECTION – must match config.h!
// ============================================================
#define DISPLAY_24    // ESP32-2432S024R/C – 2.4" CYD
//#define DISPLAY_28  // ESP32-2432S028R   – 2.8" CYD

#ifdef DISPLAY_24
  #define USER_SETUP_INFO "ESP32-2432S024R/C CYD 2.4 inch"
#else
  #define USER_SETUP_INFO "ESP32-2432S028R CYD 2.8 inch"
#endif

// ---- Driver ----
#define ILI9341_DRIVER

// ---- ESP32 SPI pins (display) ----
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST   -1  // Connected to EN on CYD

// ---- Backlight ----
// S024R/C = GPIO 27, S028R = GPIO 21
#ifdef DISPLAY_24
  #define TFT_BL    27
#else
  #define TFT_BL    21
#endif
#define TFT_BACKLIGHT_ON HIGH

// ---- Touch (XPT2046) ----
#define TOUCH_CS  33

#ifdef DISPLAY_24
  // S024: touch shares HSPI with display (same MOSI/MISO/SCLK)
#else
  // S028: touch on separate SPI bus (software SPI)
  #define TOUCH_MOSI  32
  #define TOUCH_MISO  39
  #define TOUCH_CLK   25
#endif

// ---- SPI Frequency ----
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// ---- Fonts ----
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT
