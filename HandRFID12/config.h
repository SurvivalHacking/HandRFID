// ============================================================
//  HandRFID-Touch – Hardware Configuration
//  Target boards: ESP32-2432S024R/C (2.4") / ESP32-2432S028R (2.8")
//  S024: resistive (XPT2046) + capacitive (CST820) auto-detect
//  S028: resistive only (XPT2046 on SPI bus separato)
//  Reconstructed from V2.1 firmware for S028R
// ============================================================
#pragma once

// ============================================================
//  DISPLAY SELECTION – uncomment ONE of the following
//  *** Cambiare anche in User_Setup.h (copiato nella libreria TFT_eSPI) ***
// ============================================================
#define DISPLAY_24    // ESP32-2432S024R/C – 2.4" CYD
// #define DISPLAY_28  // ESP32-2432S028R/C – 2.8" CYD

// ---- Validate selection ----
#if defined(DISPLAY_24) && defined(DISPLAY_28)
  #error "Select only ONE display: DISPLAY_24 or DISPLAY_28"
#elif !defined(DISPLAY_24) && !defined(DISPLAY_28)
  #error "Select a display: uncomment DISPLAY_24 or DISPLAY_28"
#endif

// ---------- Display (ILI9341 on HSPI) ----------
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST   -1          // tied to EN on CYD boards

// *** S024R/C vs S028R/C: backlight and I2C SDA are swapped ***
#ifdef DISPLAY_24
  #define TFT_BL    27
#else
  #define TFT_BL    21
#endif
#define TFT_BACKLIGHT_ON HIGH

// ---------- Touch – Resistive XPT2046 ----------
#define TOUCH_CS   33
#define TOUCH_IRQ  36

#ifdef DISPLAY_24
  // S024: touch condivide HSPI con il display (stessi MOSI/MISO/SCLK)
#else
  // S028: touch su bus SPI separato (pin dedicati)
  #define TOUCH_MOSI  32
  #define TOUCH_MISO  39
  #define TOUCH_CLK   25
#endif

// ---------- Touch – Capacitive CST820 (solo S024C, auto-detect) ----------
#ifdef DISPLAY_24
  #define CTP_SDA    33
  #define CTP_SCL    32
  #define CTP_RST    25
  #define CTP_INT    21
#endif

// ---------- I2C for PN532 NFC ----------
#ifdef DISPLAY_24
  #define I2C_SDA   21  //1        // S024R/C: GPIO 21
  #define I2C_SCL   22  //3        // same on both boards
#else
  #define I2C_SDA   27        // S028R/C: GPIO 27
  #define I2C_SCL   22          // same on both boards
#endif
#define PN532_I2C_ADDR 0x24

// ---------- RGB LED (active LOW on CYD) ----------
#define LED_R      4
#define LED_G     16
#define LED_B     17

// ---------- NVS namespace ----------
#define NVS_NAMESPACE "touch"

// ---------- Firmware version ----------
#ifdef DISPLAY_24
  #define FW_VERSION "1.2-S024"
#else
  #define FW_VERSION "1.2-S028"
#endif

// ---------- Display ----------
#define SCREEN_W 240
#define SCREEN_H 320

// ---------- MIFARE Classic 1K (QIDI) ----------
// QIDI usa sector 1 block 0 = absolute block 4
// Formato: byte[0]=Material code, byte[1]=Color code(1-24), byte[2]=Manufacturer(0=Generic,1=QIDI)
#define QIDI_DATA_BLOCK   4
// Keys da provare in ordine: {key[6], keyType} — keyType: 0=A, 1=B
#define QIDI_NUM_KEYS     8
struct QidiKeyEntry { uint8_t key[6]; uint8_t type; };
static const QidiKeyEntry QIDI_KEYS[QIDI_NUM_KEYS] = {
  {{0x1B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 0},  // Key A primaria QIDI
  {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1},  // Key B zeros (common)
  {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 0},  // Key A default factory
  {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 1},  // Key B default factory
  {{0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}, 0},  // Key A MAD
  {{0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7}, 0},
  {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},  // Key A zeros
  {{0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}, 0},
};

// Codici materiale QIDI (NON sequenziali!)
struct QidiMatCode { const char* name; uint8_t code; };
#define QIDI_MAT_CODE_COUNT 36
static const QidiMatCode qidiMatCodes[QIDI_MAT_CODE_COUNT] = {
  {"PLA",        1}, {"PLA Matte",  2}, {"PLA Metal",  3}, {"PLA Silk",   4},
  {"PLA-CF",     5}, {"PLA-Wood",   6}, {"PLA-Basic",   7}, {"PLA-Matte Bas",  8},
  {"ABS",       11}, {"ABS-GF",    12}, {"ABS-Metal", 13}, {"ABS-OdorLess",  14},
  {"ASA",       18}, {"ASA-AERO",  19},
  {"PA",        24}, {"PA-CF",     25}, {"UltaPA-CF25", 26}, {"PA12-CF",     27},
  {"PAHT-CF",   30}, {"PAHT-GF",   31}, {"Supp.-PATH",  32}, {"Sup.-PET/PA", 33},
  {"PC/ABS-FR", 34},
  {"PET-CF",    37}, {"PET-GF",    38}, {"PETG-Basic",  39}, {"PETG-Tough",  40},
  {"PETG",      41}, {"PETG-CF",   42}, {"PETG Rapido", 43},
  {"PPS-CF",    44}, {"PETG-TR",   45}, {"PPS-GF",      46},
  {"PVA",       47}, {"TPU-Aero",  49},
  {"TPU",       50},
};

// Palette colori condivisa Qidi/Anycubic
// rgb = colore RGB24, qidiCode = codice Qidi 1-24 (0 = solo Anycubic)
struct ColorEntry { uint32_t rgb; uint8_t qidiCode; const char* nameEN; const char* nameIT; };
#define COLOR_PALETTE_COUNT 28
static const ColorEntry colorPalette[COLOR_PALETTE_COUNT] = {
  // --- Colori Qidi (qidiCode 1-24) ---
  {0xFAFAFA,  1, "White",       "Bianco"},
  {0x060606,  2, "Black",       "Nero"},
  {0xD9E3ED,  3, "Silver",      "Argento"},
  {0x5CF30F,  4, "Light Green", "Verde chiaro"},
  {0x63E492,  5, "Mint",        "Menta"},
  {0x2850FF,  6, "Blue",        "Blu"},
  {0xFE98FE,  7, "Pink",        "Rosa"},
  {0xDFD628,  8, "Yellow",      "Giallo"},
  {0x228332,  9, "Green",       "Verde"},
  {0x99DEFF, 10, "Light Blue",  "Azzurro"},
  {0x1714B0, 11, "Dark Blue",   "Blu scuro"},
  {0xCEC0FE, 12, "Lavender",    "Lavanda"},
  {0xCADE4B, 13, "Lime",        "Lime"},
  {0x1353AB, 14, "Royal Blue",  "Blu reale"},
  {0x5EA9FD, 15, "Sky Blue",    "Blu cielo"},
  {0xA878FF, 16, "Violet",      "Viola"},
  {0xFE717A, 17, "Rose",        "Rosato"},
  {0xFF362D, 18, "Red",         "Rosso"},
  {0xE2DFCD, 19, "Beige",       "Beige"},
  {0x898F9B, 20, "Gray",        "Grigio"},
  {0x6E3812, 21, "Brown",       "Marrone"},
  {0xCAC59F, 22, "Khaki",       "Kaki"},
  {0xF28636, 23, "Orange",      "Arancione"},
  {0xB87F2B, 24, "Bronze",      "Bronzo"},
  // --- Colori extra Anycubic (qidiCode = 0, no mapping Qidi) ---
  {0x00FFFF,  0, "Cyan",        "Ciano"},
  {0x8000FF,  0, "Purple",      "Porpora"},
  {0xFF69B4,  0, "Hot Pink",    "Rosa acceso"},
  {0xFFD700,  0, "Gold",        "Oro"},
};
#define MIFARE_BLOCKS 64
#define MIFARE_BLOCK_SIZE 16

// ---------- NTAG (Anycubic) – page layout ----------
#define ACE_PAGE_HEADER     4
#define ACE_PAGE_SKU        5   // 5-8   (20 bytes, 5 pages)
#define ACE_PAGE_BRAND     10   // 10-13 (16 bytes, 4 pages)
#define ACE_PAGE_TYPE      15   // 15-18 (16 bytes, 4 pages)
#define ACE_PAGE_COLOR     20   // 20    (4 bytes ABGR)
#define ACE_PAGE_EXT_TEMP  24   // 24    (4 bytes: min16LE + max16LE)
#define ACE_PAGE_BED_TEMP  29   // 29    (4 bytes: min16LE + max16LE)
#define ACE_PAGE_FILAMENT  30   // 30    (4 bytes: diameter16LE + length16LE)
#define ACE_PAGE_UNK       31   // 31    (E8 03 00 00)
#define ACE_HEADER_MAGIC   {0x7B, 0x00, 0x65, 0x00}
#define ACE_BRAND_DEFAULT  "AC"
#define ACE_SKU_LEN        20
#define ACE_STR_LEN        16

// ---------- WiFi (WiFiManager + web editor) ----------
#define WIFI_PORTAL_SSID "HandRFID-Setup"  // nome AP per config portal
#define WIFI_PORTAL_PASS "handrfid1"       // password AP config portal
#define WIFI_PORTAL_TIMEOUT 120            // secondi timeout config portal
#define WEB_PORT         80

// ---------- SPIFFS ----------
#define CSV_PATH         "/materials.csv"

// ---------- Material/Manufacturer DB ----------
#define MAX_MATERIALS       40
#define MAX_MANUFACTURERS   20
#define MAX_ACE_MATERIALS   20
#define MAT_NVS_NS   "matdb"
#define MFG_NVS_NS   "mfgdb"
#define MAT_KEY_FMT  "m%02u"

// ---------- Default manufacturers ----------
#define DEFAULT_MFG_1 "QIDI"
#define DEFAULT_MFG_2 "Anycubic"
#define DEFAULT_MFG_3 "Generic"

// ---------- Anycubic material entry (dynamic, loaded from SPIFFS) ----------
struct AceMatEntry {
  char     name[24];
  char     sku[24];
  uint16_t extMin, extMax;
  uint16_t bedMin, bedMax;
  bool     used;
};


// Weight -> length (meters) lookup
#define ACE_WEIGHT_COUNT 5
struct AceWeightEntry { const char* label; uint16_t grams; uint16_t meters; };
static const AceWeightEntry aceWeightDB[ACE_WEIGHT_COUNT] = {
  {"250g",   250,   82},
  {"500g",   500,  165},
  {"600g",   600,  198},
  {"750g",   750,  247},
  {"1000g", 1000,  330},
};
