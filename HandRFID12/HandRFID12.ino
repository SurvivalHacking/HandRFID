// ============================================================
//  HandRFID V1.2
//  By Paolo Sambinello e Davide Gatti  www.survivalhacking.it
//  GITHUB: https://github.com/SurvivalHacking/handrfid
//
//  Dispositivo portatile per la gestione pratica dei TAG RFID delle bobine dei filamenti QIDI e ANYCUBIC / 
//  Handheld device for QIDI and ANYCUBIC reel RFID TAG 
//  * Gestione costruttori / Manufacturer management 
//  * Gestione Materiali / Material management
//  * Configurabile via WEB / Cofigurable via WEB
//  * Open Source e personalizzabile / Open source full customizable 
//
//  Elenco materiali / Bill of material
//  PN532: https://s.click.aliexpress.com/e/_c30J9g7Z (ATTENZIONE/WARNING  SW1=ON SW2=OFF)
//  ESP32-2432S024R: https://s.click.aliexpress.com/e/_c3bvlCXF
//  Batt. 803040 1000mA: https://s.click.aliexpress.com/e/_c4LtLOBT	
//
//  TAG Adesivi / adhesive TAG
//  Mifare Classic 1K: https://s.click.aliexpress.com/e/_c3QRUTPz
//  NTAG 213: https://s.click.aliexpress.com/e/_c3fpyESh
//
//  Copiare il file User_Setup.h nella cartella [utente]/documenti/arduino/libraries/TFT_eSPI	
//  Copy the file User_Setup.h on folder [users]/document/arduino/libraries/TFT_eSPI	
//
//  V1.0 15/03/2026 By Paolo Sambinello e Davide Gatti
//  Release iniziale / Initial release
//
//  V1.1 18/04/2026 by Davide Gatti
//  Sistemati alcuni problemi che facevano mostrare dati in modo non preciso. A volte mancava il costruttore e a volte il colore non aveva il nome ma un codice e non venivano rilevati correttamente i TAG vuoti
//
//  V1.2 05/06/2022 by Davide Gatti
//  Aggiunta lingua Francese
//
// ============================================================




#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_PN532.h>
#include <FS.h>
using namespace fs;        // espone File, FS nel global scope (richiesto da WebServer ESP32 core 3.x)
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include "qrcode.h"
#include <time.h>              // NTP time

#include "config.h"
#include "lang.h"
#include "utf8_latin1.h"
#include "FreeSans9pt8b.h"
#include "FreeSansBold12pt8b.h"
#include "webui.h"
#include "logo.h"
#ifdef DISPLAY_24
  #include "CST820.h"
#endif

// ---- Button struct (must be before any function using it) ----
struct Btn { int x, y, w, h; const char* label; uint16_t bg; };

// ---- Fonts (FreeSans = Arial equivalent) ----
// Inclusi tramite TFT_eSPI con LOAD_GFXFF abilitato
#define FONT_TITLE  &FreeSansBold12pt8b   // titoli / header (Latin-1 extended)
#define FONT_BODY   &FreeSans9pt8b        // testo normale (Latin-1 extended)
#define FONT_SMALL  &FreeSans9pt8b        // testo piccolo (Latin-1 extended)

// ---- Touch type ----
#ifdef DISPLAY_24
  enum TouchType { TOUCH_NONE, TOUCH_RESISTIVE, TOUCH_CAPACITIVE };
#else
  enum TouchType { TOUCH_NONE, TOUCH_RESISTIVE };  // S028: solo resistivo
#endif
TouchType touchType = TOUCH_NONE;

// ---- Globals ----
TFT_eSPI tft = TFT_eSPI();

// Font helpers (must be after tft declaration)
void setFontTitle() { tft.setFreeFont(FONT_TITLE); tft.setTextSize(1); }
void setFontBody()  { tft.setFreeFont(FONT_BODY);  tft.setTextSize(1); }
void setFontSmall() { tft.setFreeFont(FONT_SMALL); tft.setTextSize(1); }
void setFontBuiltin() { tft.setTextFont(1); tft.setTextSize(1); }

#ifdef DISPLAY_28
  // S028: touch XPT2046 su bus SPI dedicato (GPIO 32/39/25)
  SPIClass touchSPI(VSPI);
  XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
#else
  // S024: touch XPT2046 condivide HSPI col display
  XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
#endif

#ifdef DISPLAY_24
  // S024: capacitive CST820 (auto-detect)
  CST820 ctp(CTP_SDA, CTP_SCL, CTP_RST, -1);  // INT=-1 to avoid GPIO 21 conflict
#endif

Adafruit_PN532 nfc(-1, -1, &Wire);     // I2C mode on Wire (GPIO 21/22)
Preferences prefs;
WebServer server(WEB_PORT);

// ---- Dynamic material databases (loaded from SPIFFS CSV) ----
AceMatEntry aceMatList[MAX_ACE_MATERIALS];
uint8_t     aceMatCount = 0;

// ---- Debug log panel ----
#define DBG_MAX_LINES  10
#define DBG_LINE_LEN   36
char   dbgLines[DBG_MAX_LINES][DBG_LINE_LEN];
int    dbgCount = 0;    // quante righe totali scritte
int    dbgStart = 0;    // indice circolare della riga piu vecchia

// ---- State ----
bool   showDebugPanel = false;  // visibilita pannello debug (default OFF)
bool   wifiEnabled    = false;  // WiFi abilitato on-demand (default OFF)
bool   wifiConnected = false;
bool   webServerStarted = false;  // server.begin() chiamato solo dopo WiFi
Lang   curLang       = LANG_IT;
bool   displayInvert = false;
bool   autoMode      = false;
bool   nfcPresent    = false;
bool   nfcBusy       = false;
bool   hasCalib      = false;
int    calibMinX = 200, calibMaxX = 3900;
int    calibMinY = 200, calibMaxY = 3900;

// ---- Tag mode (QIDI vs Anycubic) ----
enum TagMode { TAG_QIDI, TAG_ANYCUBIC };
TagMode writeMode = TAG_QIDI;   // selected write mode
TagMode lastReadMode = TAG_QIDI; // detected on last read

// Current selection for writing (QIDI)
uint8_t selMfgIdx    = 0;   // manufacturer index
uint8_t selMatIdx    = 0;   // material index
uint8_t selColorIdx  = 0;   // color index

// Current selection for writing (Anycubic)
uint8_t selAceMatIdx    = 0;  // index into aceMatList
uint8_t selAceWeightIdx = 2;  // index into aceWeightDB (default 600g)
uint8_t selAceColorIdx  = 1;          // index into colorPalette (default Black)
String   selAceBrand    = "AC";       // "AC" = Anycubic, "GEN" = Generic

// Scroll offset (shared across list screens, reset on screen change)
int scrollOffset = 0;

// Timers
unsigned long lastTouchTime = 0;
unsigned long lastActivityTime = 0;
unsigned long screensaverTimeout = 120000;  // 2 minuti in ms (configurabile via web, in secondi)
bool   rainbowEnabled  = true;   // titolo rainbow on/off
uint8_t backlightLevel = 255;    // luminosita backlight (0-255)

// ---- UI State machine ----
enum Screen {
  SCR_MAIN,
  SCR_READ_RESULT,
  SCR_WRITE_SELECT_MFG,
  SCR_WRITE_SELECT_MAT,
  SCR_WRITE_SELECT_COLOR,
  SCR_WRITING,
  SCR_SETUP,
  SCR_LANG_SELECT,
  SCR_CALIBRATE,
  SCR_KEYBOARD,
  SCR_MFG_LIST,
  SCR_MAT_LIST,
  SCR_MFG_EDIT,
  SCR_MAT_EDIT,
  SCR_CONFIRM,
  SCR_NOTICE,
  SCR_WRITE_SELECT_MODE,    // QIDI or Anycubic
  SCR_ACE_SELECT_MFG,       // Anycubic manufacturer (AC/GEN)
  SCR_ACE_SELECT_MAT,       // Anycubic material
  SCR_ACE_SELECT_COLOR,     // Anycubic color (hex input)
  SCR_ACE_SELECT_WEIGHT,    // Anycubic spool weight
  SCR_SCREENSAVER,          // orologio screensaver dopo inattivita
  SCR_WRITE_DONE,           // post-write: ristampa o menu
};
Screen curScreen = SCR_MAIN;
Screen prevScreen = SCR_MAIN;

// ---- Manufacturer / Material DB (NVS) ----
struct MfgEntry  { char name[24]; bool used; };
struct MatEntry  {
  char     name[24];
  char     sku[24];
  uint8_t  code;       // QIDI material code (1-50)
  uint16_t extMin, extMax;
  uint16_t bedMin, bedMax;
  bool     used;
};
MfgEntry mfgDB[MAX_MANUFACTURERS];
MatEntry matDB[MAX_MATERIALS];
uint8_t  mfgCount = 0;
uint8_t  matCount = 0;

// ---- Tag data ----
uint8_t tagUID[7];
uint8_t tagUIDLen = 0;
uint8_t tagBlock[16];
bool    tagPresent = false;

// ---- Read tag decoded fields (QIDI) ----
String  readMfg, readMat, readColor;
uint8_t readMfgIdx, readMatIdx, readColorIdx;

// ---- Read tag decoded fields (Anycubic) ----
String   aceReadSku, aceReadBrand, aceReadType;
uint32_t aceReadColor = 0;          // ABGR
uint16_t aceReadExtMin, aceReadExtMax;
uint16_t aceReadBedMin, aceReadBedMax;
uint16_t aceReadDiameter, aceReadLength;


// ---- Notice ----
String noticeText;
Screen noticeReturn;

// ---- Confirm ----
String confirmText;
typedef void (*ConfirmCallback)(bool);
ConfirmCallback confirmCb = nullptr;

// ---- Forward declarations ----
void drawMainScreen();
void drawSetupScreen();
void drawLangSelect();
void drawCalibScreen();
void drawReadResult(bool showBack = true);
void drawWriteSelectMfg();
void drawWriteSelectMat();
void drawWriteSelectColor();
void drawNotice();
void drawConfirm();
void drawWriteSelectMode();
void drawAceSelectMfg();
void drawAceSelectMat();
void drawAceSelectColor();
void drawAceSelectWeight();
void drawWriteDone();
void drawReadResultAnycubic(bool showBack = true);
void handleTouch(int x, int y);
bool nfcReselect(uint8_t* uid, uint8_t* uidLen);
bool readTag();
bool readTagInternal(uint8_t* uid, uint8_t uidLen);
bool writeTag();
bool readTagAnycubic();
bool readTagAnycubicInternal(uint8_t* uid, uint8_t uidLen);
bool writeTagAnycubic();
bool readTagOnce();
bool readTagValidated();
bool readTagAuto();
void loadSettings();
void saveSettings();
void loadMfgDB();
void saveMfgDB();
void loadMatDB();
void saveMatDB();
void initDefaultMfg();
void initDefaultMat();
void setBacklight(uint8_t brightness);
void showStatus(const char* msg, uint16_t color = TFT_WHITE);
void startWebServer();
void connectWifi();
void disconnectWifi();
void ledOrange();
void ledOff();
void ledBlinkBlue();

// ================================================================
//  Backlight
// ================================================================
void setBacklight(uint8_t brightness) {
  analogWrite(TFT_BL, brightness);
}

// LED flash: lampeggia 3 volte velocemente (active LOW)
void ledFlash(uint8_t pin, int count = 3, int onMs = 80, int offMs = 80) {
  for (int i = 0; i < count; i++) {
    digitalWrite(pin, LOW);   // ON
    delay(onMs);
    digitalWrite(pin, HIGH);  // OFF
    if (i < count - 1) delay(offMs);
  }
}
void ledFlashOk()   { ledFlash(LED_G); }  // verde = successo
void ledFlashErr()  { ledFlash(LED_R); }  // rosso = errore

// ================================================================
//  NVS – Settings
// ================================================================
void loadSettings() {
  prefs.begin(NVS_NAMESPACE, true);
  curLang       = (Lang)prefs.getUChar("lang", LANG_IT);
  if (curLang >= LANG_COUNT) curLang = LANG_IT;
  displayInvert = prefs.getBool("dinv", false);
  autoMode      = prefs.getBool("auto", false);
  hasCalib      = prefs.getBool("hascal", false);
  calibMinX     = prefs.getInt("minx", 200);
  calibMaxX     = prefs.getInt("maxx", 3900);
  calibMinY     = prefs.getInt("miny", 200);
  calibMaxY     = prefs.getInt("maxy", 3900);
  screensaverTimeout = prefs.getULong("sstout", 120000);
  rainbowEnabled     = prefs.getBool("rainbow", true);
  backlightLevel     = prefs.getUChar("blight", 255);
  showDebugPanel     = prefs.getBool("dbgpanel", false);
  prefs.end();
}

void saveSettings() {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.putUChar("lang", (uint8_t)curLang);
  prefs.putBool("dinv", displayInvert);
  prefs.putBool("auto", autoMode);
  prefs.putBool("hascal", hasCalib);
  prefs.putInt("minx", calibMinX);
  prefs.putInt("maxx", calibMaxX);
  prefs.putInt("miny", calibMinY);
  prefs.putInt("maxy", calibMaxY);
  prefs.putULong("sstout", screensaverTimeout);
  prefs.putBool("rainbow", rainbowEnabled);
  prefs.putUChar("blight", backlightLevel);
  prefs.putBool("dbgpanel", showDebugPanel);
  prefs.end();
}

// ================================================================
//  NVS – Manufacturer DB
// ================================================================
void loadMfgDB() {
  prefs.begin(MFG_NVS_NS, true);
  mfgCount = 0;
  for (int i = 0; i < MAX_MANUFACTURERS; i++) {
    char key[8];
    snprintf(key, sizeof(key), "m%02u", i);
    String val = prefs.getString(key, "");
    if (val.length() > 0) {
      strncpy(mfgDB[mfgCount].name, val.c_str(), 23);
      mfgDB[mfgCount].name[23] = 0;
      mfgDB[mfgCount].used = true;
      mfgCount++;
    }
  }
  prefs.end();
  if (mfgCount == 0) initDefaultMfg();
}

void saveMfgDB() {
  prefs.begin(MFG_NVS_NS, false);
  prefs.clear();
  for (int i = 0; i < mfgCount; i++) {
    char key[8];
    snprintf(key, sizeof(key), "m%02u", i);
    prefs.putString(key, mfgDB[i].name);
  }
  prefs.end();
}

void initDefaultMfg() {
  // Clear any stale entries beyond the defaults
  for (int i = 0; i < MAX_MANUFACTURERS; i++) {
    memset(mfgDB[i].name, 0, sizeof(mfgDB[i].name));
    mfgDB[i].used = false;
  }
  mfgCount = 3;
  strncpy(mfgDB[0].name, DEFAULT_MFG_1, 23); mfgDB[0].used = true;
  strncpy(mfgDB[1].name, DEFAULT_MFG_2, 23); mfgDB[1].used = true;
  strncpy(mfgDB[2].name, DEFAULT_MFG_3, 23); mfgDB[2].used = true;
  saveMfgDB();
}

// ================================================================
//  NVS – Material DB
// ================================================================
void loadMatDB() {
  prefs.begin(MAT_NVS_NS, true);
  matCount = 0;
  for (int i = 0; i < MAX_MATERIALS; i++) {
    char key[8];
    snprintf(key, sizeof(key), MAT_KEY_FMT, i);
    String val = prefs.getString(key, "");
    if (val.length() > 0) {
      strncpy(matDB[matCount].name, val.c_str(), 23);
      matDB[matCount].name[23] = 0;
      matDB[matCount].used = true;
      matCount++;
    }
  }
  prefs.end();
  if (matCount == 0) initDefaultMat();
}

void saveMatDB() {
  prefs.begin(MAT_NVS_NS, false);
  prefs.clear();
  for (int i = 0; i < matCount; i++) {
    char key[8];
    snprintf(key, sizeof(key), MAT_KEY_FMT, i);
    prefs.putString(key, matDB[i].name);
  }
  prefs.end();
}

void initDefaultMat() {
  matCount = min((int)NUM_DEFAULT_MATERIALS, MAX_MATERIALS);
  for (int i = 0; i < matCount; i++) {
    strncpy(matDB[i].name, defaultMaterials[i], 23);
    matDB[i].name[23] = 0;
    matDB[i].used = true;
  }
  saveMatDB();
}

// ================================================================
//  Touch abstraction
//  S024: resistive XPT2046 / capacitive CST820 (auto-detect)
//  S028: resistive XPT2046 only
// ================================================================
bool touchIsTouched() {
  if (touchType == TOUCH_RESISTIVE)
    return ts.touched();
#ifdef DISPLAY_24
  if (touchType == TOUCH_CAPACITIVE) {
    uint16_t x, y; uint8_t g;
    return ctp.getTouch(&x, &y, &g);
  }
#endif
  return false;
}

void touchGetPoint(int &x, int &y) {
  if (touchType == TOUCH_RESISTIVE) {
    TS_Point p = ts.getPoint();
    x = map(p.x, calibMinX, calibMaxX, 0, SCREEN_W);
    y = map(p.y, calibMinY, calibMaxY, 0, SCREEN_H);
    x = constrain(x, 0, SCREEN_W - 1);
    y = constrain(y, 0, SCREEN_H - 1);
  }
#ifdef DISPLAY_24
  else if (touchType == TOUCH_CAPACITIVE) {
    uint16_t cx, cy; uint8_t g;
    ctp.getTouch(&cx, &cy, &g);
    // CST820 returns 240x320 natively; portrait mode — direct mapping
    x = cx;
    y = cy;
    x = constrain(x, 0, SCREEN_W - 1);
    y = constrain(y, 0, SCREEN_H - 1);
  }
#endif
}

void touchGetRaw(int &x, int &y) {
  if (touchType == TOUCH_RESISTIVE) {
    TS_Point p = ts.getPoint();
    x = p.x;
    y = p.y;
  }
#ifdef DISPLAY_24
  else if (touchType == TOUCH_CAPACITIVE) {
    uint16_t cx, cy; uint8_t g;
    ctp.getTouch(&cx, &cy, &g);
    x = cx;
    y = cy;
  }
#endif
}

// ================================================================
//  Status bar helper
// ================================================================
void showStatus(const char* msg, uint16_t color) {
  tft.fillRect(0, SCREEN_H - 28, SCREEN_W, 28, TFT_BLACK);
  tft.setTextColor(color, TFT_BLACK);
  setFontBody();
  tft.setTextDatum(MC_DATUM);
  tft.drawString(L1(msg), SCREEN_W / 2, SCREEN_H - 14);
}

// ================================================================
//  Debug log panel (main screen only)
// ================================================================
// Area del pannello: sotto il bottone Write, sopra il bottone Setup
#define DBG_X       4
#define DBG_Y       180
#define DBG_W       (SCREEN_W - 8)
#define DBG_H       (SCREEN_H - 48 - DBG_Y)   // lascia spazio per setup + info
#define DBG_LINE_H  10

void drawDebugPanel() {
  // Cornice
  tft.fillRect(DBG_X, DBG_Y, DBG_W, DBG_H, TFT_BLACK);
  tft.drawRect(DBG_X, DBG_Y, DBG_W, DBG_H, TFT_DARKGREY);

  // Titoletto
  tft.setTextFont(1); tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("LOG", DBG_X + 3, DBG_Y + 2);
  tft.drawFastHLine(DBG_X + 1, DBG_Y + 11, DBG_W - 2, TFT_DARKGREY);

  // Righe di log
  int maxVisible = (DBG_H - 14) / DBG_LINE_H;
  int total = min(dbgCount, DBG_MAX_LINES);
  int startLine = (total > maxVisible) ? total - maxVisible : 0;

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  for (int i = startLine; i < total; i++) {
    int lineIdx = (dbgStart + i) % DBG_MAX_LINES;
    int py = DBG_Y + 13 + (i - startLine) * DBG_LINE_H;
    tft.drawString(dbgLines[lineIdx], DBG_X + 3, py);
  }
}

void debugLog(const char* msg) {
  // Aggiungi al buffer circolare
  int slot;
  if (dbgCount < DBG_MAX_LINES) {
    slot = dbgCount;
  } else {
    slot = dbgStart;
    dbgStart = (dbgStart + 1) % DBG_MAX_LINES;
  }
  strncpy(dbgLines[slot], msg, DBG_LINE_LEN - 1);
  dbgLines[slot][DBG_LINE_LEN - 1] = 0;
  if (dbgCount < DBG_MAX_LINES) dbgCount++;

  // Ridisegna il pannello solo se siamo nella main screen E il pannello è abilitato
  if (curScreen == SCR_MAIN && showDebugPanel) drawDebugPanel();
}

void debugClear() {
  dbgCount = 0;
  dbgStart = 0;
  if (curScreen == SCR_MAIN && showDebugPanel) drawDebugPanel();
}

void debugLogf(const char* fmt, ...) {
  char buf[DBG_LINE_LEN];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  debugLog(buf);
}

// ================================================================
//  Button helper
// ================================================================

// ================================================================
//  Button rendering — gradiente + effetto rilievo 3D
// ================================================================

// Estrae componenti R,G,B da RGB565
inline void rgb565ToRgb(uint16_t c, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (c >> 11) << 3;
  g = ((c >> 5) & 0x3F) << 2;
  b = (c & 0x1F) << 3;
}

// Crea RGB565 da componenti (con clamp)
inline uint16_t rgbToRgb565b(int r, int g, int b) {
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Schiarisce un colore RGB565 di `amount` (0-255)
uint16_t colorLighten(uint16_t c, int amount) {
  uint8_t r, g, b;
  rgb565ToRgb(c, r, g, b);
  return rgbToRgb565b(r + amount, g + amount, b + amount);
}

// Scurisce un colore RGB565 di `amount`
uint16_t colorDarken(uint16_t c, int amount) {
  uint8_t r, g, b;
  rgb565ToRgb(c, r, g, b);
  return rgbToRgb565b(r - amount, g - amount, b - amount);
}

// Interpola tra due colori RGB565 (t=0..steps)
uint16_t colorLerp(uint16_t c1, uint16_t c2, int t, int steps) {
  uint8_t r1, g1, b1, r2, g2, b2;
  rgb565ToRgb(c1, r1, g1, b1);
  rgb565ToRgb(c2, r2, g2, b2);
  int r = r1 + (int)(r2 - r1) * t / steps;
  int g = g1 + (int)(g2 - g1) * t / steps;
  int b = b1 + (int)(b2 - b1) * t / steps;
  return rgbToRgb565b(r, g, b);
}

void drawBtn(const Btn& b) {
  const int r = 6;
  // Lookup table per offset angoli arrotondati con r=6
  // xOff[dy] = quanti pixel escludere a sinistra (e destra) per dy=0..5
  static const uint8_t cornerOff[6] = {6, 3, 2, 1, 1, 0};

  uint16_t colTop = colorLighten(b.bg, 40);
  uint16_t colBot = colorDarken(b.bg, 30);

  // Gradiente riga per riga
  for (int row = 0; row < b.h; row++) {
    uint16_t c = colorLerp(colTop, colBot, row, b.h - 1);
    int dy = min(row, b.h - 1 - row);
    int xOff = (dy < r) ? cornerOff[dy] : 0;
    tft.drawFastHLine(b.x + xOff, b.y + row, b.w - 2 * xOff, c);
  }

  // Bordo esterno scuro
  tft.drawRoundRect(b.x, b.y, b.w, b.h, r, colorDarken(b.bg, 60));
  // Highlight in alto (luce)
  tft.drawFastHLine(b.x + r, b.y + 1, b.w - 2*r, colorLighten(b.bg, 80));
  // Shadow in basso
  tft.drawFastHLine(b.x + r, b.y + b.h - 2, b.w - 2*r, colorDarken(b.bg, 70));

  // Testo con ombra sottile
  uint16_t colMid = colorLerp(colTop, colBot, b.h / 2, b.h - 1);
  tft.setTextDatum(MC_DATUM);
  setFontBody();
  const char* lbl = L1(b.label);
  int tw = tft.textWidth(lbl);
  int maxW = b.w - 8;
  if (tw > maxW) {
    setFontSmall();
    tw = tft.textWidth(lbl);
    if (tw > maxW) { tft.setTextFont(1); tft.setTextSize(1); }
  }
  tft.setTextColor(colorDarken(b.bg, 100), colMid);
  tft.drawString(lbl, b.x + b.w / 2 + 1, b.y + b.h / 2 + 1);
  tft.setTextColor(TFT_WHITE, colMid);
  tft.drawString(lbl, b.x + b.w / 2, b.y + b.h / 2);
}

bool btnHit(const Btn& b, int tx, int ty) {
  return tx >= b.x && tx < b.x + b.w && ty >= b.y && ty < b.y + b.h;
}

// Feedback visivo premuto: gradiente invertito (effetto affondato)
void btnFlash(const Btn& b) {
  const int r = 6;
  static const uint8_t cornerOff[6] = {6, 3, 2, 1, 1, 0};

  uint16_t colTop = colorDarken(b.bg, 20);
  uint16_t colBot = colorLighten(b.bg, 10);

  for (int row = 0; row < b.h; row++) {
    uint16_t c = colorLerp(colTop, colBot, row, b.h - 1);
    int dy = min(row, b.h - 1 - row);
    int xOff = (dy < r) ? cornerOff[dy] : 0;
    tft.drawFastHLine(b.x + xOff, b.y + row, b.w - 2 * xOff, c);
  }

  tft.drawRoundRect(b.x, b.y, b.w, b.h, r, colorDarken(b.bg, 80));
  tft.drawFastHLine(b.x + r, b.y + 1, b.w - 2*r, colorDarken(b.bg, 50));

  uint16_t colMid = colorLerp(colTop, colBot, b.h / 2, b.h - 1);
  tft.setTextDatum(MC_DATUM);
  setFontBody();
  const char* lbl2 = L1(b.label);
  tft.setTextColor(colorDarken(b.bg, 100), colMid);
  tft.drawString(lbl2, b.x + b.w / 2 + 1, b.y + b.h / 2 + 1);
  tft.setTextColor(TFT_WHITE, colMid);
  tft.drawString(lbl2, b.x + b.w / 2, b.y + b.h / 2);
}

// Feedback visivo per cella colore (senza testo)
void cellFlash(int cx, int cy, int cw, int ch) {
  tft.drawRoundRect(cx, cy, cw, ch, 4, TFT_YELLOW);
  tft.drawRoundRect(cx - 1, cy - 1, cw + 2, ch + 2, 4, TFT_YELLOW);
}

// ================================================================
//  MAIN SCREEN
// ================================================================
#define BTN_H    38
#define BTN_GAP  5
#define HDR_H    32

// Scroll arrow buttons (drawn at top-right and bottom-right of list area)
#define ARROW_W  44
#define ARROW_H  32
Btn btnScrollUp, btnScrollDn;

void drawScrollArrows(bool canUp, bool canDown, int bottomReserve = 80) {
  int ax = SCREEN_W - ARROW_W - 6;
  if (canUp) {
    btnScrollUp = {ax, HDR_H + 2, ARROW_W, ARROW_H, "UP", TFT_BLUE};
    drawBtn(btnScrollUp);
  } else {
    btnScrollUp = {-100, -100, 0, 0, "", TFT_BLACK};
  }
  if (canDown) {
    btnScrollDn = {ax, SCREEN_H - bottomReserve - ARROW_H - 4, ARROW_W, ARROW_H, "DN", TFT_BLUE};
    drawBtn(btnScrollDn);
  } else {
    btnScrollDn = {-100, -100, 0, 0, "", TFT_BLACK};
  }
}

Btn btnRead, btnWrite, btnSetup, btnStopAuto;

// Nomi giorni abbreviati
// ---- Rainbow title ----
uint16_t rainbowHueOffset = 0;
unsigned long lastRainbowUpdate = 0;

// HSV to RGB565 (h=0..359, s=0..255, v=0..255)
uint16_t hsvToRgb565(uint16_t h, uint8_t s, uint8_t v) {
  uint8_t r, g, b;
  uint8_t region = h / 60;
  uint8_t remainder = (h % 60) * 255 / 60;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  switch (region) {
    case 0:  r = v; g = t; b = p; break;
    case 1:  r = q; g = v; b = p; break;
    case 2:  r = p; g = v; b = t; break;
    case 3:  r = p; g = q; b = v; break;
    case 4:  r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Disegna "HandRFID" con ogni lettera di un colore arcobaleno diverso
void drawRainbowTitle() {
  const char* title = "HandRFID";
  int len = strlen(title);
  setFontTitle();
  tft.setTextDatum(ML_DATUM);
  // Calcola larghezza totale per centrare
  int totalW = 0;
  for (int i = 0; i < len; i++) {
    char ch[2] = {title[i], 0};
    totalW += tft.textWidth(ch);
  }
  int x = (SCREEN_W - totalW) / 2;
  int y = 18;
  for (int i = 0; i < len; i++) {
    uint16_t hue = (rainbowHueOffset + i * 360 / len) % 360;
    tft.setTextColor(hsvToRgb565(hue, 255, 255), TFT_BLACK);
    char ch[2] = {title[i], 0};
    tft.drawString(L1(ch), x, y);
    x += tft.textWidth(ch);
  }
}

void drawMainScreen() {
  curScreen = SCR_MAIN;
  tft.fillScreen(TFT_BLACK);

  // Title
  if (rainbowEnabled) {
    drawRainbowTitle();
  } else {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    setFontTitle();
    tft.drawString("HandRFID", SCREEN_W / 2, 18);
  }

  int bw = SCREEN_W - 20;

  int btnBottomH = 28;
  int btnBottomY = SCREEN_H - btnBottomH - 4;
  // Riserva spazio per IP WiFi sopra i pulsanti bottom (solo se WiFi connesso)
  int ipReserve = wifiConnected ? 14 : 0;
  int availH = btnBottomY - ipReserve - 44 - 8;

  int bigH;
  if (showDebugPanel) {
    bigH = 60;
  } else {
    bigH = (availH - 8) / 2;
  }

  // Pulsante LEGGI TAG (o LEGGI TAG AUTO se auto mode attivo)
  int y = 44;
  const char* readLabel = autoMode ? strAutoOn[curLang] : strReadTag[curLang];
  uint16_t readColor = autoMode ? TFT_DARKGREEN : TFT_NAVY;
  btnRead  = {10, y, bw, bigH, readLabel, readColor};
  drawBtn(btnRead);
  y += bigH + 8;

  // Pulsante SCRIVI TAG
  btnWrite = {10, y, bw, bigH, strWriteTag[curLang], TFT_NAVY};
  drawBtn(btnWrite);

  // NFC status solo se errore
  if (!nfcPresent) {
    showStatus("NFC: N/A", TFT_RED);
  }

  // Debug log panel (solo se abilitato)
  if (showDebugPanel) drawDebugPanel();

  // Pulsanti Setup e Auto in basso + IP a destra
  // btnBottomH e btnBottomY già calcolati sopra
  int btnBottomW = (SCREEN_W - 24) / 2;  // due pulsanti affiancati

  btnSetup = {4, btnBottomY, btnBottomW, btnBottomH, "Setup", TFT_DARKGREY};
  drawBtn(btnSetup);

  int toggleX = 4 + btnBottomW + 4;
  if (autoMode) {
    btnStopAuto = {toggleX, btnBottomY, btnBottomW, btnBottomH, strStopAuto[curLang], TFT_RED};
  } else {
    btnStopAuto = {toggleX, btnBottomY, btnBottomW, btnBottomH, "AUTO", TFT_DARKGREEN};
  }
  drawBtn(btnStopAuto);

  // IP del dispositivo sopra i pulsanti bottom
  if (wifiConnected) {
    setFontBuiltin();
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("IP:" + WiFi.localIP().toString(), SCREEN_W / 2, btnBottomY - 5);
  }
}

// ================================================================
//  AUTO-LEARN: aggiunge materiale al DB se non esiste
// ================================================================

// Cerca se un materiale Anycubic esiste gia nel DB, ritorna indice o -1
int aceMatFind(const char* name) {
  for (int i = 0; i < aceMatCount; i++) {
    if (strcmp(aceMatList[i].name, name) == 0) return i;
  }
  return -1;
}

// Aggiunge materiale Anycubic letto dal tag se non esiste
void aceAutoLearn(const String& type, const String& sku,
                  uint16_t eMin, uint16_t eMax, uint16_t bMin, uint16_t bMax) {
  if (type.length() == 0) return;
  if (aceMatFind(type.c_str()) >= 0) return;  // gia presente
  if (aceMatCount >= MAX_ACE_MATERIALS) return; // DB pieno

  strncpy(aceMatList[aceMatCount].name, type.c_str(), 23);
  aceMatList[aceMatCount].name[23] = 0;
  strncpy(aceMatList[aceMatCount].sku, sku.c_str(), 23);
  aceMatList[aceMatCount].sku[23] = 0;
  aceMatList[aceMatCount].extMin = eMin;
  aceMatList[aceMatCount].extMax = eMax;
  aceMatList[aceMatCount].bedMin = bMin;
  aceMatList[aceMatCount].bedMax = bMax;
  aceMatList[aceMatCount].used = true;
  aceMatCount++;
  saveMaterialsCSV();
  Serial.printf("Auto-learn Anycubic: %s\n", type.c_str());
}

// Cerca se un materiale QIDI esiste gia nel DB, ritorna indice o -1
int qidiMatFind(const char* name) {
  for (int i = 0; i < matCount; i++) {
    if (strcmp(matDB[i].name, name) == 0) return i;
  }
  return -1;
}

// ================================================================
//  READ TAG
// ================================================================
bool readTag() {
  nfcBusy = true;
  debugLog("Scanning MIFARE Classic...");
  showStatus(strWaiting[curLang], TFT_YELLOW);
  ledBlinkBlue();

  uint8_t uid[7]; uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 3000);
  ledOff();
  if (!found) {
    debugLog("No tag found");
    showStatus(strNoTag[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  char uidStr[24];
  snprintf(uidStr, sizeof(uidStr), "UID: %02X:%02X:%02X:%02X", uid[0], uid[1], uid[2], uid[3]);
  debugLog(uidStr);
  memcpy(tagUID, uid, uidLen);
  tagUIDLen = uidLen;

  debugLog("Authenticating block 4...");
  if (!readTagInternal(uid, uidLen)) {
    debugLog("Auth FAILED");
    showStatus(strAuthFail[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  debugLog("Read OK - QIDI tag");
  showStatus(strClassicDetected[curLang], TFT_GREEN);
  ledOff(); ledFlashOk(); ledOff();
  nfcBusy = false;
  return true;
}

// Helper: ri-seleziona tag dopo auth fallita (delay + readPassiveTargetID)
bool nfcReselect(uint8_t* uid, uint8_t* uidLen) {
  delay(50);
  return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLen, 1000);
}

bool readTagInternal(uint8_t* uid, uint8_t uidLen) {
  // Authenticate block 4 trying all key/type combinations
  bool authed = false;
  for (int k = 0; k < QIDI_NUM_KEYS && !authed; k++) {
    uint8_t key[6];
    memcpy(key, QIDI_KEYS[k].key, 6);
    uint8_t kt = QIDI_KEYS[k].type;
    if (k > 0) {
      uint8_t reUid[7]; uint8_t reLen;
      if (!nfcReselect(reUid, &reLen)) continue;
      memcpy(uid, reUid, reLen);
      uidLen = reLen;
    }
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLen, QIDI_DATA_BLOCK, kt, key)) {
      authed = true;
      debugLogf("Auth OK: key%c #%d", kt ? 'B' : 'A', k);
      Serial.printf("Auth OK: key%c #%d\n", kt ? 'B' : 'A', k);
    }
  }
  if (!authed) return false;

  debugLog("Reading block 4...");
  if (!nfc.mifareclassic_ReadDataBlock(QIDI_DATA_BLOCK, tagBlock)) {
    debugLog("Block read FAILED");
    return false;
  }

  // Detect format: ASCII text (byte[0] >= 'A') vs QIDI numeric codes
  if (tagBlock[0] >= 0x41 && tagBlock[0] <= 0x7A) {
    // ASCII format: material name stored as text string
    char matStr[17] = {0};
    memcpy(matStr, tagBlock, 16);
    matStr[16] = 0;
    readMat = String(matStr);
    readMat.trim();
    // Se la stringa è vuota, tag invalido
    if (readMat.length() == 0) {
      debugLog("ASCII: empty string -> invalid");
      lastReadMode = TAG_QIDI;
      tagPresent = false;
      return true;  // tag letto fisicamente, ma dati vuoti
    }
    readMatIdx = 0;
    // Try to match by name to get the index
    for (int i = 0; i < QIDI_MAT_CODE_COUNT; i++) {
      if (readMat.startsWith(qidiMatCodes[i].name)) {
        readMatIdx = i;
        break;
      }
    }
    readColor = "?";
    readColorIdx = 0;
    readMfgIdx = 0;
    readMfg = "Generic";
    debugLogf("ASCII: %s", matStr);
    Serial.printf("ASCII format detected: \"%s\"\n", matStr);
  } else {
    // QIDI numeric format (block 4):
    // byte[0] = Material code, byte[1] = Color code, byte[2] = Manufacturer
    uint8_t matCode = tagBlock[0];
    uint8_t colCode = tagBlock[1];
    uint8_t mfgCode = tagBlock[2];

    // Tag vuoto/non programmato: tutti zero o matCode=0 senza match
    bool matFound = false;
    for (int i = 0; i < QIDI_MAT_CODE_COUNT; i++) {
      if (qidiMatCodes[i].code == matCode) { matFound = true; break; }
    }
    if (matCode == 0 || (!matFound && colCode == 0)) {
      debugLogf("QIDI: empty/invalid data %02X %02X %02X -> unknown", matCode, colCode, mfgCode);
      lastReadMode = TAG_QIDI;
      tagPresent = false;
      return true;  // tag letto ma dati non validi
    }

    readMat = "Unknown(" + String(matCode) + ")";
    readMatIdx = 0;
    for (int i = 0; i < QIDI_MAT_CODE_COUNT; i++) {
      if (qidiMatCodes[i].code == matCode) {
        readMat = qidiMatCodes[i].name;
        readMatIdx = i;
        break;
      }
    }

    readColor = "?";
    readColorIdx = 0;
    for (int i = 0; i < COLOR_PALETTE_COUNT; i++) {
      if (colorPalette[i].qidiCode == colCode) {
        readColor = (curLang == LANG_IT) ? colorPalette[i].nameIT : colorPalette[i].nameEN;
        readColorIdx = i;
        break;
      }
    }

    readMfgIdx = mfgCode;
    readMfg = (mfgCode == 1) ? "QIDI" : "Generic";
    debugLogf("Data: %02X %02X %02X", tagBlock[0], tagBlock[1], tagBlock[2]);
    debugLogf("%s | %s | %s", readMfg.c_str(), readMat.c_str(), readColor.c_str());
  }

  lastReadMode = TAG_QIDI;
  tagPresent = true;
  return true;
}

void drawReadResult(bool showBack) {
  curScreen = SCR_READ_RESULT;
  tft.fillScreen(TFT_BLACK);

  int btnBottomY = showBack ? (SCREEN_H - 44) : SCREEN_H;
  int cx = SCREEN_W / 2;

  // ── Riga 1: TAG QIDI (tipo tag, sempre azzurro grande) ──
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString("TAG QIDI", cx, 18);

  // ── Riga 2: Produttore + Materiale (bianco grande) ──
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setFontTitle();
  String mfgMatLabel = readMfg + " " + readMat;
  mfgMatLabel.toUpperCase();
  tft.drawString(L1(mfgMatLabel), cx, 44);

  // ── Riga 3: Rettangolo colore grande centrato ──
  int swY = 62;
  int swW = SCREEN_W - 40;
  int swH = 36;
  uint16_t c565 = TFT_DARKGREY;
  if (readColorIdx < COLOR_PALETTE_COUNT)
    c565 = rgbToRgb565(colorPalette[readColorIdx].rgb);
  tft.fillRoundRect((SCREEN_W - swW) / 2, swY, swW, swH, 6, c565);
  tft.drawRoundRect((SCREEN_W - swW) / 2, swY, swW, swH, 6, TFT_WHITE);

  // ── Riga 4: Nome colore centrato ──
  int y = swY + swH + 10;
  setFontBody();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(L1(readColor), cx, y);
  y += 20;

  // ── Spazio ──
  y += 6;

  // ── Righe temperatura: giustificate a sinistra ──
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  for (int i = 0; i < QIDI_MAT_CODE_COUNT; i++) {
    if (qidiMatCodes[i].code == tagBlock[0]) {
      for (int j = 0; j < matCount; j++) {
        if (strcmp(matDB[j].name, qidiMatCodes[i].name) == 0 && matDB[j].extMax > 0) {
          char tmp[40];
          snprintf(tmp, sizeof(tmp), "%s: %d-%d \xb0C", strExtruder[curLang], matDB[j].extMin, matDB[j].extMax);
          tft.drawString(L1(tmp), 10, y); y += 20;
          snprintf(tmp, sizeof(tmp), "%s: %d-%d \xb0C", strBed[curLang], matDB[j].bedMin, matDB[j].bedMax);
          tft.drawString(L1(tmp), 10, y); y += 20;
          break;
        }
      }
      break;
    }
  }

  // ── Back button ──
  if (showBack) {
    Btn btnBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_NAVY};
    drawBtn(btnBack);
  }
}

// ================================================================
//  WRITE TAG
// ================================================================
void drawWriteSelectMfg() {
  curScreen = SCR_WRITE_SELECT_MFG;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strManufacturer[curLang]), SCREEN_W / 2, 16);

  // Solo QIDI e Generic per tag QIDI
  int bw = SCREEN_W - 20;
  int y = HDR_H + BTN_GAP + 10;
  int bigH = BTN_H + 6;
  Btn bQidi = {10, y, bw, bigH, "QIDI", TFT_NAVY};
  drawBtn(bQidi);
  y += bigH + 10;
  Btn bGen = {10, y, bw, bigH, "Generic", TFT_NAVY};
  drawBtn(bGen);

  Btn bBack = {10, SCREEN_H - 40, bw, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

void drawWriteSelectMat() {
  curScreen = SCR_WRITE_SELECT_MAT;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strSelectMaterial[curLang]), SCREEN_W / 2, 16);

  int bw = (SCREEN_W - ARROW_W - 34) / 2;
  int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
  int maxVisible = maxRows * 2;
  int totalRows = (QIDI_MAT_CODE_COUNT + 1) / 2;
  if (scrollOffset > max(0, totalRows - maxRows)) scrollOffset = max(0, totalRows - maxRows);

  int col = 0, row = 0;
  int startItem = scrollOffset * 2;
  for (int i = startItem; i < min(QIDI_MAT_CODE_COUNT, startItem + maxVisible); i++) {
    int bx = 10 + col * (bw + 10);
    int by = HDR_H + 4 + row * BTN_H;
    Btn b = {bx, by, bw, BTN_H - 4, qidiMatCodes[i].name, TFT_NAVY};
    drawBtn(b);
    col++;
    if (col >= 2) { col = 0; row++; }
  }

  drawScrollArrows(scrollOffset > 0, scrollOffset + maxRows < totalRows, 44);

  Btn bBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

void drawWriteSelectColor() {
  curScreen = SCR_WRITE_SELECT_COLOR;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // Header: show selected material name
  setFontTitle();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(L1(strSelectColor[curLang]), SCREEN_W / 2, 10);
  setFontSmall();
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(L1(qidiMatCodes[selMatIdx].name), SCREEN_W / 2, 26);

  // Draw color grid with names
  int cols = 4;
  int cellW = (SCREEN_W - 20) / cols;
  int cellH = 34;
  int startY = HDR_H + 8;

  // Mostra solo i colori con qidiCode valido (solo quadrato colorato, senza testo)
  int shown = 0;
  for (int i = 0; i < COLOR_PALETTE_COUNT; i++) {
    if (colorPalette[i].qidiCode == 0) continue;
    int col = shown % cols;
    int row = shown / cols;
    int cx = 10 + col * cellW;
    int cy = startY + row * cellH;
    uint16_t c565 = rgbToRgb565(colorPalette[i].rgb);
    tft.fillRoundRect(cx + 1, cy + 1, cellW - 2, cellH - 2, 4, c565);
    tft.drawRoundRect(cx + 1, cy + 1, cellW - 2, cellH - 2, 4, TFT_WHITE);
    shown++;
  }

  Btn bBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

bool writeTag() {
  nfcBusy = true;
  showStatus(strWaiting[curLang], TFT_YELLOW);
  ledBlinkBlue();

  uint8_t uid[7]; uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 3000);
  ledOff();
  if (!found) {
    showStatus(strNoTag[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }

  // Authenticate with all key/type combinations
  bool authed = false;
  for (int k = 0; k < QIDI_NUM_KEYS && !authed; k++) {
    uint8_t key[6];
    memcpy(key, QIDI_KEYS[k].key, 6);
    uint8_t kt = QIDI_KEYS[k].type;
    if (k > 0) {
      uint8_t reUid[7]; uint8_t reLen;
      nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reLen, 500);
    }
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLen, QIDI_DATA_BLOCK, kt, key)) {
      authed = true;
      Serial.printf("Write auth OK: key%c #%d\n", kt ? 'B' : 'A', k);
    }
  }
  if (!authed) {
    showStatus(strAuthFail[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }

  // Prepare QIDI data using correct codes from lookup tables
  uint8_t writeData[16] = {0};
  writeData[0] = (selMatIdx < QIDI_MAT_CODE_COUNT) ? qidiMatCodes[selMatIdx].code : 1;
  writeData[1] = (selColorIdx < COLOR_PALETTE_COUNT && colorPalette[selColorIdx].qidiCode > 0)
                   ? colorPalette[selColorIdx].qidiCode : 1;
  writeData[2] = selMfgIdx;  // 0=Generic, 1=QIDI

  Serial.printf("Writing block %d: %02X %02X %02X ...\n", QIDI_DATA_BLOCK, writeData[0], writeData[1], writeData[2]);
  if (!nfc.mifareclassic_WriteDataBlock(QIDI_DATA_BLOCK, writeData)) {
    Serial.println("WriteDataBlock returned false");
    showStatus(strWriteFail[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  Serial.println("WriteDataBlock returned true");

  // Verify: ri-seleziona, ri-autentica e rileggi
  delay(50);
  uint8_t reUid[7]; uint8_t reLen;
  if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reLen, 1000)) {
    Serial.println("Verify: re-select failed");
    showStatus("Write verify FAIL!", TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  // Ri-autentica con tutte le chiavi
  bool vAuthed = false;
  for (int k = 0; k < QIDI_NUM_KEYS && !vAuthed; k++) {
    uint8_t vkey[6]; memcpy(vkey, QIDI_KEYS[k].key, 6);
    if (nfc.mifareclassic_AuthenticateBlock(reUid, reLen, QIDI_DATA_BLOCK, QIDI_KEYS[k].type, vkey)) {
      vAuthed = true;
    } else if (k < QIDI_NUM_KEYS - 1) {
      nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reLen, 500);
    }
  }
  if (!vAuthed) {
    Serial.println("Verify: re-auth failed");
    showStatus("Write verify FAIL!", TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  uint8_t verifyBuf[16] = {0};
  if (!nfc.mifareclassic_ReadDataBlock(QIDI_DATA_BLOCK, verifyBuf)) {
    Serial.println("Verify: read failed");
    showStatus("Write verify FAIL!", TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  if (memcmp(writeData, verifyBuf, 16) != 0) {
    Serial.println("VERIFY MISMATCH!");
    Serial.printf("  Written: %02X %02X %02X\n", writeData[0], writeData[1], writeData[2]);
    Serial.printf("  Read:    %02X %02X %02X\n", verifyBuf[0], verifyBuf[1], verifyBuf[2]);
    showStatus("Write verify FAIL!", TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  Serial.println("Write verified OK!");

  showStatus(strWriteOk[curLang], TFT_GREEN);
  ledOff(); ledFlashOk(); ledOff();
  nfcBusy = false;
  return true;
}

// ================================================================
//  ANYCUBIC (NTAG) – READ / WRITE
// ================================================================

// Helper: write a string padded to len bytes across NTAG pages
void aceWritePages(uint8_t startPage, const char* str, uint8_t len) {
  uint8_t buf[24] = {0};
  strncpy((char*)buf, str, len);
  for (uint8_t i = 0; i < len / 4; i++) {
    nfc.ntag2xx_WritePage(startPage + i, &buf[i * 4]);
  }
}

// Helper: read pages into buffer
bool aceReadPages(uint8_t startPage, uint8_t* buf, uint8_t len) {
  // NTAG read returns 4 bytes per page
  for (uint8_t i = 0; i < len / 4; i++) {
    uint8_t page[4];
    if (!nfc.ntag2xx_ReadPage(startPage + i, page)) return false;
    memcpy(&buf[i * 4], page, 4);
  }
  return true;
}

// Helper: extract null-terminated string from buffer
String aceExtractString(uint8_t* buf, uint8_t maxLen) {
  char tmp[25] = {0};
  uint8_t n = min(maxLen, (uint8_t)24);
  memcpy(tmp, buf, n);
  tmp[n] = 0;
  return String(tmp);
}

bool readTagAnycubic() {
  nfcBusy = true;
  debugLog("Scanning NTAG...");
  showStatus(strWaiting[curLang], TFT_YELLOW);
  ledBlinkBlue();

  uint8_t uid[7]; uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 3000);
  ledOff();
  if (!found) {
    debugLog("No tag found");
    showStatus(strNoTag[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  char uidStr[28];
  snprintf(uidStr, sizeof(uidStr), "UID: %02X:%02X:%02X:%02X:%02X:%02X:%02X",
           uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
  debugLog(uidStr);
  memcpy(tagUID, uid, uidLen);
  tagUIDLen = uidLen;

  debugLog("Reading NTAG pages...");
  if (!readTagAnycubicInternal(uid, uidLen)) {
    debugLog("NTAG read FAILED");
    showStatus(strReadFail[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }
  debugLog("Read OK - Anycubic tag");
  showStatus(strNtagDetected[curLang], TFT_GREEN);
  ledOff(); ledFlashOk(); ledOff();
  nfcBusy = false;
  return true;
}

// Internal: read Anycubic NTAG with already-selected uid (no UI side effects)
bool readTagAnycubicInternal(uint8_t* uid, uint8_t uidLen) {
  // Read SKU (pages 5-9, 20 bytes)
  uint8_t buf[24] = {0};
  if (!aceReadPages(ACE_PAGE_SKU, buf, 20)) return false;
  aceReadSku = aceExtractString(buf, 20);

  // Brand (pages 10-13, 16 bytes)
  memset(buf, 0, sizeof(buf));
  aceReadPages(ACE_PAGE_BRAND, buf, 16);
  aceReadBrand = aceExtractString(buf, 16);

  // Material type (pages 15-18, 16 bytes)
  memset(buf, 0, sizeof(buf));
  aceReadPages(ACE_PAGE_TYPE, buf, 16);
  aceReadType = aceExtractString(buf, 16);

  // Color (page 20, 4 bytes ABGR)
  uint8_t cbuf[4] = {0};
  nfc.ntag2xx_ReadPage(ACE_PAGE_COLOR, cbuf);
  aceReadColor = ((uint32_t)cbuf[0] << 24) | ((uint32_t)cbuf[1] << 16) |
                 ((uint32_t)cbuf[2] << 8)  | cbuf[3];

  // Extruder temp (page 24, 4 bytes: minLE + maxLE)
  uint8_t tbuf[4] = {0};
  nfc.ntag2xx_ReadPage(ACE_PAGE_EXT_TEMP, tbuf);
  aceReadExtMin = tbuf[0] | (tbuf[1] << 8);
  aceReadExtMax = tbuf[2] | (tbuf[3] << 8);

  // Bed temp (page 29)
  nfc.ntag2xx_ReadPage(ACE_PAGE_BED_TEMP, tbuf);
  aceReadBedMin = tbuf[0] | (tbuf[1] << 8);
  aceReadBedMax = tbuf[2] | (tbuf[3] << 8);

  // Filament (page 30: diameter + length)
  nfc.ntag2xx_ReadPage(ACE_PAGE_FILAMENT, tbuf);
  aceReadDiameter = tbuf[0] | (tbuf[1] << 8);
  aceReadLength   = tbuf[2] | (tbuf[3] << 8);

  debugLogf("%s | %s", aceReadBrand.c_str(), aceReadType.c_str());
  debugLogf("SKU: %s", aceReadSku.c_str());

  // Validazione: la chiave è lo SKU. Se lo SKU è presente il tag è valido.
  // Brand mancante → Generic. Tipo mancante → lascia "?" ma accetta.
  bool skuOk = (aceReadSku.length() > 0);

  if (!skuOk) {
    debugLogf("Anycubic: no SKU -> unknown tag (brand='%s' type='%s')",
              aceReadBrand.c_str(), aceReadType.c_str());
    lastReadMode = TAG_ANYCUBIC;
    tagPresent = false;
    return true;  // tag letto fisicamente, ma dati non validi
  }

  // Brand mancante: imposta Generic
  if (aceReadBrand.length() == 0) {
    aceReadBrand = "GEN";
    debugLog("Anycubic: brand missing, defaulting to Generic");
  }

  // Auto-learn: se il materiale letto non esiste nel DB Anycubic, aggiungilo
  aceAutoLearn(aceReadType, aceReadSku,
               aceReadExtMin, aceReadExtMax, aceReadBedMin, aceReadBedMax);

  lastReadMode = TAG_ANYCUBIC;
  tagPresent = true;
  return true;
}

bool writeTagAnycubic() {
  nfcBusy = true;
  showStatus(strWaiting[curLang], TFT_YELLOW);
  ledBlinkBlue();

  uint8_t uid[7]; uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 3000);
  ledOff();
  if (!found) {
    showStatus(strNoTag[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    nfcBusy = false;
    return false;
  }

  const AceMatEntry& mat = aceMatList[selAceMatIdx];
  const AceWeightEntry& wt = aceWeightDB[selAceWeightIdx];

  // Header (page 4)
  uint8_t hdr[] = ACE_HEADER_MAGIC;
  nfc.ntag2xx_WritePage(ACE_PAGE_HEADER, hdr);

  // SKU (pages 5-9, 20 bytes)
  aceWritePages(ACE_PAGE_SKU, mat.sku, 20);

  // Brand (pages 10-13, 16 bytes)
  aceWritePages(ACE_PAGE_BRAND, selAceBrand.c_str(), 16);

  // Type (pages 15-18, 16 bytes)
  aceWritePages(ACE_PAGE_TYPE, mat.name, 16);

  // Color (page 20, 4 bytes ABGR) - converti da palette RGB
  uint32_t abgr = rgbToAbgr(colorPalette[selAceColorIdx].rgb);
  uint8_t cbuf[4];
  cbuf[0] = (abgr >> 24) & 0xFF;  // A
  cbuf[1] = (abgr >> 16) & 0xFF;  // B
  cbuf[2] = (abgr >> 8)  & 0xFF;  // G
  cbuf[3] =  abgr        & 0xFF;  // R
  nfc.ntag2xx_WritePage(ACE_PAGE_COLOR, cbuf);

  // Extruder temp (page 24)
  uint8_t tbuf[4];
  tbuf[0] = mat.extMin & 0xFF; tbuf[1] = (mat.extMin >> 8) & 0xFF;
  tbuf[2] = mat.extMax & 0xFF; tbuf[3] = (mat.extMax >> 8) & 0xFF;
  nfc.ntag2xx_WritePage(ACE_PAGE_EXT_TEMP, tbuf);

  // Bed temp (page 29)
  tbuf[0] = mat.bedMin & 0xFF; tbuf[1] = (mat.bedMin >> 8) & 0xFF;
  tbuf[2] = mat.bedMax & 0xFF; tbuf[3] = (mat.bedMax >> 8) & 0xFF;
  nfc.ntag2xx_WritePage(ACE_PAGE_BED_TEMP, tbuf);

  // Filament (page 30: diameter=175 i.e. 1.75mm*100, length from weight table)
  uint16_t diam = 175;
  tbuf[0] = diam & 0xFF; tbuf[1] = (diam >> 8) & 0xFF;
  tbuf[2] = wt.meters & 0xFF; tbuf[3] = (wt.meters >> 8) & 0xFF;
  nfc.ntag2xx_WritePage(ACE_PAGE_FILAMENT, tbuf);

  // Unknown page 31
  uint8_t unk[] = {0xE8, 0x03, 0x00, 0x00};
  nfc.ntag2xx_WritePage(ACE_PAGE_UNK, unk);

  showStatus(strWriteOk[curLang], TFT_GREEN);
  ledOff(); ledFlashOk(); ledOff();
  nfcBusy = false;
  return true;
}

// Lettura singola (senza validazione) — ritorna true se riesce
bool readTagOnce() {
  uint8_t uid[7]; uint8_t uidLen;
  bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 1000);
  if (!found) return false;
  memcpy(tagUID, uid, uidLen);
  tagUIDLen = uidLen;

  if (uidLen == 4) {
    debugLog("UID 4B -> MIFARE Classic");
    return readTagInternal(uid, uidLen);
  } else if (uidLen == 7) {
    debugLog("UID 7B -> NTAG");
    return readTagAnycubicInternal(uid, uidLen);
  }
  return false;
}

// Lettura validata: 2 letture consecutive uguali
bool readTagValidated() {
  nfcBusy = true;
  debugLog("Validated read...");
  showStatus(strWaiting[curLang], TFT_YELLOW);

  uint8_t prevBlock[16] = {0};
  int matchCount = 0;
  int failCount = 0;

  for (int attempt = 0; attempt < 6; attempt++) {
    ledBlinkBlue();
    debugLogf("Attempt %d/6...", attempt + 1);
    if (!readTagOnce()) {
      failCount++;
      if (failCount > 3) {
        debugLog("Too many failures");
        showStatus(strNoTag[curLang], TFT_RED);
        ledOff();
        ledOff(); ledFlashErr(); ledOff();
        ledOff();
        nfcBusy = false;
        return false;
      }
      delay(200);
      continue;
    }

    // Confronta con lettura precedente
    if (matchCount == 0) {
      memcpy(prevBlock, tagBlock, 16);
      matchCount = 1;
    } else if (memcmp(prevBlock, tagBlock, 16) == 0) {
      matchCount++;
      if (matchCount >= 2) {
        debugLog("2x match - data valid");
        ledOff();
        nfcBusy = false;
        return true;
      }
    } else {
      debugLog("Mismatch, retry...");
      memcpy(prevBlock, tagBlock, 16);
      matchCount = 1;
    }
    delay(100);
  }

  debugLog("Validation failed");
  showStatus(strReadFail[curLang], TFT_RED);
  ledOff();
  ledOff(); ledFlashErr(); ledOff();
  ledOff();
  nfcBusy = false;
  return false;
}

bool readTagAuto() {
  // Azzera UID prima di ogni tentativo
  memset(tagUID, 0, sizeof(tagUID));
  tagUIDLen = 0;
  tagPresent = false;

  if (readTagValidated()) {
    // Tag letto con successo dal punto di vista NFC
    if (!tagPresent) {
      // Tag fisicamente presente e leggibile, ma dati non validi/vuoti
      // Mostra schermata TAG SCONOSCIUTO
      curScreen = SCR_READ_RESULT;
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      setFontTitle();
      tft.drawString(L1(curLang == LANG_IT ? "TAG SCONOSCIUTO" : "UNKNOWN TAG"), SCREEN_W / 2, SCREEN_H / 2 - 20);
      setFontBody();
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.drawString(L1(curLang == LANG_IT ? "Dati non validi o tag vuoto" : "Invalid data or blank tag"), SCREEN_W / 2, SCREEN_H / 2 + 10);
      Btn btnBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_NAVY};
      drawBtn(btnBack);
      ledOff(); ledFlashErr(); ledOff();
      return false;
    }
    return true;
  }

  // LED sicuramente spento a questo punto (readTagValidated lo garantisce)
  // readTagValidated ha già aggiornato tagUID se ha trovato qualcosa.
  // Se UID è tutto zero/vuoto = nessun tag fisico presente
  bool hasValidUID = false;
  if (tagUIDLen > 0) {
    for (uint8_t i = 0; i < tagUIDLen; i++) {
      if (tagUID[i] != 0) { hasValidUID = true; break; }
    }
  }

  if (!hasValidUID) {
    showStatus(strNoTag[curLang], TFT_RED);
    ledOff(); ledFlashErr(); ledOff();
    delay(1200);
    drawMainScreen();
  } else {
    // Tag fisicamente presente ma tipo non riconosciuto (né QIDI né Anycubic)
    // Mostra schermata TAG NON RICONOSCIUTO
    curScreen = SCR_READ_RESULT;
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    setFontTitle();
    tft.drawString(L1(curLang == LANG_IT ? "TAG NON RICONOSCIUTO" : "UNKNOWN TAG"), SCREEN_W / 2, SCREEN_H / 2 - 20);
    setFontBody();
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(L1(curLang == LANG_IT ? "Tipo tag non supportato" : "Unsupported tag type"), SCREEN_W / 2, SCREEN_H / 2 + 10);
    Btn btnBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_NAVY};
    drawBtn(btnBack);
    ledOff(); ledFlashErr(); ledOff();
  }
  return false;
}

// ================================================================
//  ANYCUBIC SCREENS
// ================================================================

void drawWriteSelectMode() {
  curScreen = SCR_WRITE_SELECT_MODE;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strSelectMode[curLang]), SCREEN_W / 2, 16);

  int bw = SCREEN_W - 20;
  int y = HDR_H + BTN_GAP + 10;
  Btn bQidi = {10, y, bw, BTN_H + 6, strQidiTag[curLang], TFT_NAVY};
  drawBtn(bQidi);
  y += BTN_H + 16;
  Btn bAce  = {10, y, bw, BTN_H + 6, strAnycubicTag[curLang], TFT_NAVY};
  drawBtn(bAce);

  Btn bBack = {10, SCREEN_H - 40, bw, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

void drawAceSelectMfg() {
  curScreen = SCR_ACE_SELECT_MFG;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strManufacturer[curLang]), SCREEN_W / 2, 16);

  int bw = SCREEN_W - 20;
  int y = HDR_H + BTN_GAP + 10;
  int bigH = BTN_H + 6;
  Btn bAce = {10, y, bw, bigH, "Anycubic", TFT_NAVY};
  drawBtn(bAce);
  y += bigH + 10;
  Btn bGen = {10, y, bw, bigH, "Generic", TFT_NAVY};
  drawBtn(bGen);

  Btn bBack = {10, SCREEN_H - 40, bw, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

void drawAceSelectMat() {
  curScreen = SCR_ACE_SELECT_MAT;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strSelectMaterial[curLang]), SCREEN_W / 2, 16);

  int bw = (SCREEN_W - ARROW_W - 34) / 2;
  int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
  int maxVisible = maxRows * 2;
  int totalRows = ((int)aceMatCount + 1) / 2;
  if (scrollOffset > max(0, totalRows - maxRows)) scrollOffset = max(0, totalRows - maxRows);

  int col = 0, row = 0;
  int startItem = scrollOffset * 2;
  for (int i = startItem; i < min((int)aceMatCount, startItem + maxVisible); i++) {
    int bx = 10 + col * (bw + 10);
    int by = HDR_H + 4 + row * BTN_H;
    Btn b = {bx, by, bw, BTN_H - 4, aceMatList[i].name, TFT_NAVY};
    drawBtn(b);
    col++;
    if (col >= 2) { col = 0; row++; }
  }

  drawScrollArrows(scrollOffset > 0, scrollOffset + maxRows < totalRows, 44);

  Btn bBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

// Anycubic usa tutta la palette condivisa (COLOR_PALETTE_COUNT colori)

void drawAceSelectColor() {
  curScreen = SCR_ACE_SELECT_COLOR;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // Header: show selected material name
  setFontTitle();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(L1(strSelectColor[curLang]), SCREEN_W / 2, 10);
  setFontSmall();
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(L1(aceMatList[selAceMatIdx].name), SCREEN_W / 2, 26);

  int cols = 4;
  int cellW = (SCREEN_W - 20) / cols;
  int cellH = 34;
  int startY = HDR_H + 8;
  // Solo quadrati colorati, senza testo
  for (int i = 0; i < COLOR_PALETTE_COUNT; i++) {
    int col = i % cols;
    int row = i / cols;
    int cx = 10 + col * cellW;
    int cy = startY + row * cellH;
    uint16_t c565 = rgbToRgb565(colorPalette[i].rgb);
    tft.fillRoundRect(cx + 1, cy + 1, cellW - 2, cellH - 2, 4, c565);
    tft.drawRoundRect(cx + 1, cy + 1, cellW - 2, cellH - 2, 4, TFT_WHITE);
  }

  Btn bBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

void drawAceSelectWeight() {
  curScreen = SCR_ACE_SELECT_WEIGHT;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(L1(strSelectWeight[curLang]), SCREEN_W / 2, 10);

  // Summary bar: material name + color swatch + color name
  int sumY = 26;
  const ColorEntry& ce = colorPalette[selAceColorIdx];
  uint16_t c565 = rgbToRgb565(ce.rgb);
  const char* colName = (curLang == LANG_IT) ? ce.nameIT : ce.nameEN;
  setFontSmall();
  // Material name
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(L1(aceMatList[selAceMatIdx].name), SCREEN_W / 2 - 14, sumY);
  // Color swatch (small square)
  tft.fillRoundRect(SCREEN_W / 2 - 10, sumY - 8, 16, 16, 3, c565);
  tft.drawRoundRect(SCREEN_W / 2 - 10, sumY - 8, 16, 16, 3, TFT_WHITE);
  // Color name
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(L1(colName), SCREEN_W / 2 + 10, sumY);
  tft.setTextDatum(MC_DATUM);

  // Weight buttons — 2 columns, shifted down to make room for summary
  int topY = HDR_H + 18;
  int bw = (SCREEN_W - 30) / 2;
  int bh = BTN_H;
  int col = 0, row = 0;
  for (int i = 0; i < ACE_WEIGHT_COUNT; i++) {
    int bx = 10 + col * (bw + 10);
    int by = topY + row * (bh + 4);
    Btn b = {bx, by, bw, bh - 4, aceWeightDB[i].label, TFT_NAVY};
    drawBtn(b);
    col++;
    if (col >= 2) { col = 0; row++; }
  }

  Btn bBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_MAROON};
  drawBtn(bBack);
}

// ABGR -> RGB565 for TFT display
uint16_t abgrToRgb565(uint32_t abgr) {
  uint8_t r = abgr & 0xFF;
  uint8_t g = (abgr >> 8) & 0xFF;
  uint8_t b = (abgr >> 16) & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Converte RGB24 -> RGB565 per TFT
uint16_t rgbToRgb565(uint32_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Converte RGB24 -> ABGR32 per tag Anycubic (alpha = 0xFF)
uint32_t rgbToAbgr(uint32_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
  return 0xFF000000 | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
}

void drawReadResultAnycubic(bool showBack) {
  curScreen = SCR_READ_RESULT;
  tft.fillScreen(TFT_BLACK);

  int cx = SCREEN_W / 2;

  // ── Riga 1: TAG ANYCUBIC (tipo tag, sempre azzurro grande) ──
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString("TAG ANYCUBIC", cx, 18);

  // ── Riga 2: Produttore + Tipo materiale (bianco grande) ──
  String mfgName;
  if (aceReadBrand == "AC")        mfgName = "ANYCUBIC";
  else if (aceReadBrand == "GEN")  mfgName = "GENERIC";
  else                             mfgName = aceReadBrand;
  mfgName.toUpperCase();
  String typeName = aceReadType;
  typeName.toUpperCase();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setFontTitle();
  tft.drawString(L1(mfgName + " " + typeName), cx, 44);

  // ── Riga 3: Rettangolo colore grande centrato ──
  int swY = 62;
  int swW = SCREEN_W - 40;
  int swH = 36;
  uint16_t tftCol = abgrToRgb565(aceReadColor);
  tft.fillRoundRect((SCREEN_W - swW) / 2, swY, swW, swH, 6, tftCol);
  tft.drawRoundRect((SCREEN_W - swW) / 2, swY, swW, swH, 6, TFT_WHITE);

  // ── Riga 4: Nome colore — cerca il più vicino nella palette (distanza euclidea) ──
  int y = swY + swH + 10;
  setFontBody();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  uint8_t aceR = aceReadColor & 0xFF;
  uint8_t aceG = (aceReadColor >> 8) & 0xFF;
  uint8_t aceB = (aceReadColor >> 16) & 0xFF;
  String colorName = "";
  long bestDist = 0x7FFFFFFF;
  int bestIdx = -1;
  for (int i = 0; i < COLOR_PALETTE_COUNT; i++) {
    uint8_t pR = (colorPalette[i].rgb >> 16) & 0xFF;
    uint8_t pG = (colorPalette[i].rgb >> 8)  & 0xFF;
    uint8_t pB =  colorPalette[i].rgb        & 0xFF;
    long dr = (long)aceR - pR, dg = (long)aceG - pG, db = (long)aceB - pB;
    long dist = dr*dr + dg*dg + db*db;
    if (dist < bestDist) { bestDist = dist; bestIdx = i; }
  }
  // Accetta il match solo se la distanza è ragionevole (< 50^2*3 = 7500)
  if (bestIdx >= 0 && bestDist < 7500) {
    colorName = (curLang == LANG_IT) ? colorPalette[bestIdx].nameIT : colorPalette[bestIdx].nameEN;
  } else {
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", aceR, aceG, aceB);
    colorName = hex;
  }
  tft.drawString(L1(colorName), cx, y);
  y += 20;

  // ── Spazio ──
  y += 6;

  // ── Righe info: giustificate a sinistra ──
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  char tmp[40];
  if (aceReadExtMax > 0) {
    snprintf(tmp, sizeof(tmp), "%s: %d-%d \xb0C", strExtruder[curLang], aceReadExtMin, aceReadExtMax);
    tft.drawString(L1(tmp), 10, y); y += 20;
  }
  if (aceReadBedMax > 0) {
    snprintf(tmp, sizeof(tmp), "%s: %d-%d \xb0C", strBed[curLang], aceReadBedMin, aceReadBedMax);
    tft.drawString(L1(tmp), 10, y); y += 20;
  }
  if (aceReadSku.length()) {
    tft.drawString(L1("SKU: " + aceReadSku), 10, y); y += 20;
  }

  // ── Back button ──
  if (showBack) {
    Btn btnBack = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, strBack[curLang], TFT_NAVY};
    drawBtn(btnBack);
  }
}

// ================================================================
//  SETUP SCREEN
// ================================================================
Btn btnLang, btnCalib, btnResetWifi, btnWifiToggle, btnResetMat, btnSetupBack;

void drawSetupScreen() {
  curScreen = SCR_SETUP;
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strSetup[curLang]), SCREEN_W / 2, 16);

  int bw = SCREEN_W - 20;
  // Contiamo i "slot" di pulsanti: Lang, [Calib], WiFi-row(=1 slot), ResetMat, Back
  int numBtns = (touchType == TOUCH_RESISTIVE) ? 5 : 4;
  int totalH = SCREEN_H - HDR_H - 8;
  int gap = 6;
  int bh = (totalH - (numBtns - 1) * gap) / numBtns;
  int y = HDR_H + 4;

  btnLang = {10, y, bw, bh, strLanguage[curLang], TFT_NAVY}; y += bh + gap;
  if (touchType == TOUCH_RESISTIVE) {
    btnCalib = {10, y, bw, bh, strCalibrate[curLang], TFT_NAVY}; y += bh + gap;
  } else {
    btnCalib = {-100, -100, 0, 0, "", TFT_BLACK};
  }

  // Riga WiFi: due pulsanti affiancati (metà larghezza ciascuno)
  int halfW = (bw - 6) / 2;
  btnResetWifi  = {10,             y, halfW, bh, strResetWifi[curLang], TFT_NAVY};
  const char* wifiToggleLabel = wifiConnected
    ? strDisconnectWifi[curLang]
    : strConnectWifi[curLang];
  uint16_t wifiToggleColor = wifiConnected ? TFT_MAROON : TFT_DARKGREEN;
  btnWifiToggle = {10 + halfW + 6, y, halfW, bh, wifiToggleLabel, wifiToggleColor};
  y += bh + gap;

  btnResetMat  = {10, y, bw, bh, strResetMaterials[curLang], TFT_MAROON}; y += bh + gap;
  btnSetupBack = {10, y, bw, bh, strBack[curLang], TFT_NAVY};

  drawBtn(btnLang);
  if (touchType == TOUCH_RESISTIVE) drawBtn(btnCalib);
  drawBtn(btnResetWifi);
  drawBtn(btnWifiToggle);
  drawBtn(btnResetMat);
  drawBtn(btnSetupBack);
}

// ================================================================
//  LANGUAGE SELECT
// ================================================================
void drawLangSelect() {
  curScreen = SCR_LANG_SELECT;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strSelectLang[curLang]), SCREEN_W / 2, 16);

  int y = HDR_H + 2;
  int bw = SCREEN_W - 20;
  int bh = 30;
  int gap = 3;
  // 6 languages * 33px = 198px, starting at y=32, ending at y=230 → fits in 320
  for (int i = 0; i < LANG_COUNT; i++) {
    uint16_t bg = (i == (int)curLang) ? TFT_DARKGREEN : TFT_NAVY;
    Btn b = {10, y, bw, bh, langNames[i], bg};
    drawBtn(b);
    y += bh + gap;
  }
}

// ================================================================
//  CALIBRATION
// ================================================================
void drawCalibScreen() {
  curScreen = SCR_CALIBRATE;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontBody();
  tft.drawString(L1(strCalibInstr[curLang]), SCREEN_W / 2, SCREEN_H / 2);

  // Draw crosshairs at corners
  int margin = 20;
  uint16_t cc = TFT_YELLOW;
  // Top-left
  tft.drawLine(margin - 10, margin, margin + 10, margin, cc);
  tft.drawLine(margin, margin - 10, margin, margin + 10, cc);
  // Top-right
  tft.drawLine(SCREEN_W - margin - 10, margin, SCREEN_W - margin + 10, margin, cc);
  tft.drawLine(SCREEN_W - margin, margin - 10, SCREEN_W - margin, margin + 10, cc);
  // Bottom-left
  tft.drawLine(margin - 10, SCREEN_H - margin, margin + 10, SCREEN_H - margin, cc);
  tft.drawLine(margin, SCREEN_H - margin - 10, margin, SCREEN_H - margin + 10, cc);
  // Bottom-right
  tft.drawLine(SCREEN_W - margin - 10, SCREEN_H - margin, SCREEN_W - margin + 10, SCREEN_H - margin, cc);
  tft.drawLine(SCREEN_W - margin, SCREEN_H - margin - 10, SCREEN_W - margin, SCREEN_H - margin + 10, cc);
}

void runCalibration() {
#ifdef DISPLAY_24
  // Capacitive touch doesn't need calibration
  if (touchType == TOUCH_CAPACITIVE) {
    hasCalib = true;
    showStatus("Capacitive: no calibration needed", TFT_GREEN);
    delay(1500);
    return;
  }
#endif

  drawCalibScreen();

  int rawX[4], rawY[4];
  int margin = 20;
  int screenPts[4][2] = {
    {margin, margin},
    {SCREEN_W - margin, margin},
    {margin, SCREEN_H - margin},
    {SCREEN_W - margin, SCREEN_H - margin}
  };

  for (int pt = 0; pt < 4; pt++) {
    // Highlight current point
    tft.fillCircle(screenPts[pt][0], screenPts[pt][1], 6, TFT_RED);

    // Show progress
    char progStr[8];
    snprintf(progStr, sizeof(progStr), "%d/4", pt + 1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(L1(progStr), SCREEN_W / 2, SCREEN_H / 2 + 20);

    // Wait for stable touch — collect multiple samples and average
    unsigned long touchStart = 0;
    bool touching = false;
    while (true) {
      if (touchIsTouched()) {
        if (!touching) {
          touchStart = millis();
          touching = true;
        }
        if (millis() - touchStart > 800) {
          // Collect 16 samples, discard min/max 4, average the rest
          const int nSamples = 16;
          int sx[nSamples], sy[nSamples];
          for (int s = 0; s < nSamples; s++) {
            int rx, ry;
            touchGetRaw(rx, ry);
            sx[s] = rx;
            sy[s] = ry;
            delay(15);
          }
          // Simple sort for trimmed mean
          for (int i = 0; i < nSamples - 1; i++)
            for (int j = i + 1; j < nSamples; j++) {
              if (sx[j] < sx[i]) { int t = sx[i]; sx[i] = sx[j]; sx[j] = t; }
              if (sy[j] < sy[i]) { int t = sy[i]; sy[i] = sy[j]; sy[j] = t; }
            }
          // Average middle 8 samples (discard 4 lowest + 4 highest)
          long sumX = 0, sumY = 0;
          for (int s = 4; s < 12; s++) { sumX += sx[s]; sumY += sy[s]; }
          rawX[pt] = sumX / 8;
          rawY[pt] = sumY / 8;
          break;
        }
      } else {
        touching = false;
      }
      delay(10);
    }

    // Wait for release
    while (touchIsTouched()) delay(10);
    tft.fillCircle(screenPts[pt][0], screenPts[pt][1], 6, TFT_GREEN);
    delay(300);
  }

  // Calculate calibration — extrapolate from margin to full screen edges
  int rawAtMinX = (rawX[0] + rawX[2]) / 2;   // raw X at screen x=margin
  int rawAtMaxX = (rawX[1] + rawX[3]) / 2;   // raw X at screen x=SCREEN_W-margin
  int rawAtMinY = (rawY[0] + rawY[1]) / 2;   // raw Y at screen y=margin
  int rawAtMaxY = (rawY[2] + rawY[3]) / 2;   // raw Y at screen y=SCREEN_H-margin

  // Extrapolate: the calib points are at 'margin' from edges, not at 0/max
  float scaleX = (float)(rawAtMaxX - rawAtMinX) / (SCREEN_W - 2 * margin);
  float scaleY = (float)(rawAtMaxY - rawAtMinY) / (SCREEN_H - 2 * margin);
  calibMinX = rawAtMinX - (int)(margin * scaleX);
  calibMaxX = rawAtMaxX + (int)(margin * scaleX);
  calibMinY = rawAtMinY - (int)(margin * scaleY);
  calibMaxY = rawAtMaxY + (int)(margin * scaleY);

  // Ensure min < max
  if (calibMinX > calibMaxX) { int t = calibMinX; calibMinX = calibMaxX; calibMaxX = t; }
  if (calibMinY > calibMaxY) { int t = calibMinY; calibMinY = calibMaxY; calibMaxY = t; }

  hasCalib = true;
  saveSettings();
  showStatus(strCalibSaved[curLang], TFT_GREEN);
  delay(1500);
  drawMainScreen();
}

// ================================================================
//  NOTICE / CONFIRM
// ================================================================
void drawNotice() {
  curScreen = SCR_NOTICE;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strPleaseNote[curLang]), SCREEN_W / 2, 16);

  setFontBody();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Word-wrap the notice text
  int y = 44;
  int idx = 0;
  while (idx < (int)noticeText.length() && y < SCREEN_H - 50) {
    int nl = noticeText.indexOf('\n', idx);
    String line;
    if (nl >= 0) {
      line = noticeText.substring(idx, nl);
      idx = nl + 1;
    } else {
      line = noticeText.substring(idx);
      idx = noticeText.length();
    }
    tft.drawString(L1(line), SCREEN_W / 2, y);
    y += 16;
  }

  Btn bOk = {10, SCREEN_H - 40, SCREEN_W - 20, BTN_H, "OK", TFT_NAVY};
  drawBtn(bOk);
}

void drawConfirm() {
  curScreen = SCR_CONFIRM;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strConfirm[curLang]), SCREEN_W / 2, 16);

  setFontBody();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = 50;
  int idx = 0;
  while (idx < (int)confirmText.length()) {
    int nl = confirmText.indexOf('\n', idx);
    String line;
    if (nl >= 0) {
      line = confirmText.substring(idx, nl);
      idx = nl + 1;
    } else {
      line = confirmText.substring(idx);
      idx = confirmText.length();
    }
    tft.drawString(L1(line), SCREEN_W / 2, y);
    y += 16;
  }

  Btn bYes = {10,             SCREEN_H - 40, (SCREEN_W - 30) / 2, BTN_H, strYes[curLang], TFT_DARKGREEN};
  Btn bNo  = {SCREEN_W/2 + 5, SCREEN_H - 40, (SCREEN_W - 30) / 2, BTN_H, strNo[curLang],  TFT_MAROON};
  drawBtn(bYes);
  drawBtn(bNo);
}

// ================================================================
//  POST-WRITE SCREEN: Reprint / Menu
// ================================================================
void drawWriteDone() {
  curScreen = SCR_WRITE_DONE;
  tft.fillScreen(TFT_BLACK);

  // Title
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.drawString(L1(strWriteDoneTitle[curLang]), SCREEN_W / 2, 24);

  // Ask message
  setFontBody();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = 60;
  String msg = strWriteDoneAsk[curLang];
  int idx = 0;
  while (idx < (int)msg.length()) {
    int nl = msg.indexOf('\n', idx);
    String line;
    if (nl >= 0) { line = msg.substring(idx, nl); idx = nl + 1; }
    else { line = msg.substring(idx); idx = msg.length(); }
    tft.drawString(L1(line), SCREEN_W / 2, y);
    y += 18;
  }

  // Buttons: Reprint (left) and Menu (right)
  int bw = (SCREEN_W - 30) / 2;
  int by = SCREEN_H - 50;
  Btn bReprint = {10,             by, bw, BTN_H + 6, strReprint[curLang], TFT_DARKGREEN};
  Btn bMenu    = {SCREEN_W/2 + 5, by, bw, BTN_H + 6, strMenu[curLang],   TFT_MAROON};
  drawBtn(bReprint);
  drawBtn(bMenu);
}

// ================================================================
//  TOUCH HANDLER
// ================================================================
void handleTouch(int tx, int ty) {
  switch (curScreen) {
    case SCR_MAIN:
      if (btnHit(btnRead, tx, ty)) {
        if (!nfcPresent) { showStatus(strNfcError[curLang], TFT_RED); return; }
        btnFlash(btnRead);
        debugClear();
        // Long press 5s = toggle auto mode
        unsigned long pressStart = millis();
        while (touchIsTouched() && (millis() - pressStart) < 5000) {
          delay(50);
        }
        if (millis() - pressStart >= 5000) {
          // Long press: toggle auto
          autoMode = !autoMode;
          saveSettings();
          drawMainScreen();
          return;
        }
        // Short press: evidenzia pulsante arancione durante lettura
        {
          Btn btnReadOrange = btnRead;
          btnReadOrange.bg = 0xFC00;  // TFT_ORANGE (RGB565)
          drawBtn(btnReadOrange);
        }
        // read tag — azzera buffer prima di iniziare
        memset(tagUID, 0, sizeof(tagUID));
        tagUIDLen = 0;
        memset(tagBlock, 0, sizeof(tagBlock));
        tagPresent = false;
        bool readOk = readTagAuto();
        if (readOk) {
          delay(500);
          if (lastReadMode == TAG_ANYCUBIC) drawReadResultAnycubic();
          else drawReadResult();
        } else {
          // Se readTagAuto ha già impostato SCR_READ_RESULT (tag sconosciuto/vuoto),
          // NON sovrascrivere con drawMainScreen — lasciare la schermata visibile
          if (curScreen != SCR_READ_RESULT) {
            drawMainScreen();
          }
        }
      }
      else if (btnHit(btnWrite, tx, ty)) {
        if (!nfcPresent) { showStatus(strNfcError[curLang], TFT_RED); return; }
        btnFlash(btnWrite);
        scrollOffset = 0;
        drawWriteSelectMode();
      }
      else if (btnHit(btnStopAuto, tx, ty)) {
        autoMode = !autoMode;
        saveSettings();
        drawMainScreen();
      }
      else if (btnHit(btnSetup, tx, ty)) {
        drawSetupScreen();
      }
      break;

    case SCR_READ_RESULT:
      // Back button is at bottom
      if (ty > SCREEN_H - 44) {
        drawMainScreen();
      }
      break;

    case SCR_WRITE_SELECT_MFG: {
      if (ty > SCREEN_H - 44) { drawWriteSelectMode(); return; }
      int y0 = HDR_H + BTN_GAP + 10;
      int btnH = BTN_H + 6;
      int bw = SCREEN_W - 20;
      // QIDI = manufacturer 1
      if (ty >= y0 && ty < y0 + btnH) {
        Btn bHit = {10, y0, bw, btnH, "QIDI", TFT_NAVY};
        btnFlash(bHit);
        selMfgIdx = 1;
        scrollOffset = 0;
        drawWriteSelectMat();
      }
      // Generic = manufacturer 0
      else if (ty >= y0 + btnH + 10 && ty < y0 + 2 * btnH + 10) {
        Btn bHit = {10, y0 + btnH + 10, bw, btnH, "Generic", TFT_NAVY};
        btnFlash(bHit);
        selMfgIdx = 0;
        scrollOffset = 0;
        drawWriteSelectMat();
      }
      break;
    }

    case SCR_WRITE_SELECT_MAT: {
      if (btnHit(btnScrollUp, tx, ty)) {
        int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
        scrollOffset = max(0, scrollOffset - maxRows);
        drawWriteSelectMat(); return;
      }
      if (btnHit(btnScrollDn, tx, ty)) {
        int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
        int totalRows = (QIDI_MAT_CODE_COUNT + 1) / 2;
        scrollOffset = min(max(0, totalRows - maxRows), scrollOffset + maxRows);
        drawWriteSelectMat(); return;
      }
      if (ty > SCREEN_H - 44) { scrollOffset = 0; drawWriteSelectMfg(); return; }
      // Only process if touch is inside the button grid area
      int mbw = (SCREEN_W - ARROW_W - 34) / 2;
      int gridTop = HDR_H + 4;
      int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
      int gridBot = gridTop + maxRows * BTN_H;
      if (ty >= gridTop && ty < gridBot && tx >= 10 && tx < 10 + 2 * mbw + 10) {
        int col = (tx > SCREEN_W / 2) ? 1 : 0;
        int row = (ty - gridTop) / BTN_H;
        int idx = (row + scrollOffset) * 2 + col;
        if (idx >= 0 && idx < QIDI_MAT_CODE_COUNT) {
          Btn bHit = {10 + col * (mbw + 10), gridTop + row * BTN_H, mbw, BTN_H - 4, qidiMatCodes[idx].name, TFT_NAVY};
          btnFlash(bHit);
          selMatIdx = idx;
          scrollOffset = 0;
          drawWriteSelectColor();
        }
      }
      break;
    }

    case SCR_WRITE_SELECT_COLOR: {
      if (ty > SCREEN_H - 44) { drawWriteSelectMat(); return; }
      int cols = 4;
      int cellW = (SCREEN_W - 20) / cols;
      int cellH = 34;
      int startY = HDR_H + 8;
      // Count how many Qidi colors we have
      int qidiColorCount = 0;
      for (int i = 0; i < COLOR_PALETTE_COUNT; i++)
        if (colorPalette[i].qidiCode != 0) qidiColorCount++;
      int totalRows = (qidiColorCount + cols - 1) / cols;
      int gridBot = startY + totalRows * cellH;
      // Only process touches inside the color grid
      if (ty < startY || ty >= gridBot || tx < 10 || tx >= 10 + cols * cellW) break;
      int col = (tx - 10) / cellW;
      int row = (ty - startY) / cellH;
      int gridIdx = row * cols + col;
      // Mappa gridIdx -> indice palette (solo colori con qidiCode > 0)
      int paletteIdx = -1, cnt = 0;
      for (int i = 0; i < COLOR_PALETTE_COUNT; i++) {
        if (colorPalette[i].qidiCode == 0) continue;
        if (cnt == gridIdx) { paletteIdx = i; break; }
        cnt++;
      }
      if (paletteIdx >= 0) {
        cellFlash(10 + col * cellW + 1, startY + row * cellH + 1, cellW - 2, cellH - 2);
        selColorIdx = paletteIdx;
        writeTag();
        delay(1500);
        drawWriteDone();
      }
      break;
    }

    case SCR_WRITE_SELECT_MODE: {
      if (ty > SCREEN_H - 44) { drawMainScreen(); return; }
      int y0 = HDR_H + BTN_GAP + 10;
      int btnH = BTN_H + 6;
      int bw = SCREEN_W - 20;
      // QIDI button
      if (ty >= y0 && ty < y0 + btnH) {
        Btn bHit = {10, y0, bw, btnH, strQidiTag[curLang], TFT_NAVY};
        btnFlash(bHit);
        writeMode = TAG_QIDI;
        scrollOffset = 0;
        drawWriteSelectMfg();
      }
      // Anycubic button
      else if (ty >= y0 + btnH + 10 && ty < y0 + 2 * btnH + 10) {
        Btn bHit = {10, y0 + btnH + 10, bw, btnH, strAnycubicTag[curLang], TFT_NAVY};
        btnFlash(bHit);
        writeMode = TAG_ANYCUBIC;
        drawAceSelectMfg();
      }
      break;
    }

    case SCR_ACE_SELECT_MFG: {
      if (ty > SCREEN_H - 44) { drawWriteSelectMode(); return; }
      int y0 = HDR_H + BTN_GAP + 10;
      int btnH = BTN_H + 6;
      int bw = SCREEN_W - 20;
      if (ty >= y0 && ty < y0 + btnH) {
        Btn bHit = {10, y0, bw, btnH, "Anycubic", TFT_NAVY};
        btnFlash(bHit);
        selAceBrand = "AC";
        scrollOffset = 0;
        drawAceSelectMat();
      }
      else if (ty >= y0 + btnH + 10 && ty < y0 + 2 * btnH + 10) {
        Btn bHit = {10, y0 + btnH + 10, bw, btnH, "Generic", TFT_NAVY};
        btnFlash(bHit);
        selAceBrand = "GEN";
        scrollOffset = 0;
        drawAceSelectMat();
      }
      break;
    }

    case SCR_ACE_SELECT_MAT: {
      if (btnHit(btnScrollUp, tx, ty)) {
        int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
        scrollOffset = max(0, scrollOffset - maxRows);
        drawAceSelectMat(); return;
      }
      if (btnHit(btnScrollDn, tx, ty)) {
        int maxRows = (SCREEN_H - HDR_H - 50) / BTN_H;
        int totalRows = ((int)aceMatCount + 1) / 2;
        scrollOffset = min(max(0, totalRows - maxRows), scrollOffset + maxRows);
        drawAceSelectMat(); return;
      }
      if (ty > SCREEN_H - 44) { scrollOffset = 0; drawWriteSelectMode(); return; }
      int col = (tx > SCREEN_W / 2) ? 1 : 0;
      int row = (ty - HDR_H - 4) / BTN_H;
      int idx = (row + scrollOffset) * 2 + col;
      if (idx >= 0 && idx < (int)aceMatCount) {
        int mbw = (SCREEN_W - ARROW_W - 34) / 2;
        Btn bHit = {10 + col * (mbw + 10), HDR_H + 4 + row * BTN_H, mbw, BTN_H - 4, aceMatList[idx].name, TFT_NAVY};
        btnFlash(bHit);
        selAceMatIdx = idx;
        drawAceSelectColor();
      }
      break;
    }

    case SCR_ACE_SELECT_COLOR: {
      if (ty > SCREEN_H - 44) { drawAceSelectMat(); return; }
      int cols = 4;
      int cellW = (SCREEN_W - 20) / cols;
      int cellH = 34;
      int startY = HDR_H + 8;
      int totalRows = (COLOR_PALETTE_COUNT + cols - 1) / cols;
      int gridBot = startY + totalRows * cellH;
      // Only process touches inside the color grid
      if (ty < startY || ty >= gridBot || tx < 10 || tx >= 10 + cols * cellW) break;
      int col = (tx - 10) / cellW;
      int row = (ty - startY) / cellH;
      int idx = row * cols + col;
      if (idx >= 0 && idx < COLOR_PALETTE_COUNT) {
        cellFlash(10 + col * cellW + 1, startY + row * cellH + 1, cellW - 2, cellH - 2);
        selAceColorIdx = idx;
        drawAceSelectWeight();
      }
      break;
    }

    case SCR_ACE_SELECT_WEIGHT: {
      if (ty > SCREEN_H - 44) { drawAceSelectColor(); return; }
      int bh = BTN_H;
      int topY = HDR_H + 18;  // matches drawAceSelectWeight layout
      int col = (tx > SCREEN_W / 2) ? 1 : 0;
      int row = (ty - topY) / (bh + 4);
      if (row < 0) break;
      int idx = row * 2 + col;
      if (idx >= 0 && idx < ACE_WEIGHT_COUNT) {
        int wbw = (SCREEN_W - 30) / 2;
        Btn bHit = {10 + col * (wbw + 10), topY + row * (bh + 4), wbw, bh - 4, aceWeightDB[idx].label, TFT_NAVY};
        btnFlash(bHit);
        selAceWeightIdx = idx;
        writeTagAnycubic();
        delay(1500);
        drawWriteDone();
      }
      break;
    }

    case SCR_SETUP:
      if (btnHit(btnLang, tx, ty))        drawLangSelect();
      else if (btnHit(btnCalib, tx, ty))   runCalibration();
      else if (btnHit(btnResetWifi, tx, ty)) {
        confirmText = String(strResetWifiConfirm[curLang]);
        confirmCb = [](bool yes) {
          if (yes) {
            WiFiManager wm;
            wm.resetSettings();
            wifiConnected = false;
            wifiEnabled   = false;
            showStatus("WiFi reset. Rebooting...", TFT_YELLOW);
            delay(1500);
            ESP.restart();
          }
          drawSetupScreen();
        };
        drawConfirm();
      }
      else if (btnHit(btnWifiToggle, tx, ty)) {
        if (wifiConnected) {
          // Scollega WiFi
          disconnectWifi();
          showStatus(strWifiDisconnected[curLang], TFT_YELLOW);
          delay(1000);
          drawSetupScreen();
        } else {
          // Collega WiFi (con LED arancione)
          connectWifi();
          drawSetupScreen();
        }
      }
      else if (btnHit(btnResetMat, tx, ty)) {
        confirmText = String(strResetMatConfirm[curLang]);
        confirmCb = [](bool yes) {
          if (yes) {
            // Cancella CSV e ricrea SOLO i default
            // I tuoi dati personalizzati vengono persi
            SPIFFS.remove(CSV_PATH);
            createDefaultCSV();
            loadMaterialsCSV();
            initDefaultMfg();
            saveMatDB();
            saveMfgDB();
            showStatus(strFactoryRestored[curLang], TFT_GREEN);
            delay(1500);
          }
          drawMainScreen();
        };
        drawConfirm();
      }
      else if (btnHit(btnSetupBack, tx, ty)) drawMainScreen();
      break;

    case SCR_LANG_SELECT: {
      // bh=30, gap=3, startY = HDR_H + 2 = 32
      int idx = (ty - (HDR_H + 2)) / 33;
      if (idx >= 0 && idx < LANG_COUNT) {
        curLang = (Lang)idx;
        saveSettings();
        drawMainScreen();
      }
      break;
    }

    case SCR_NOTICE:
      if (ty > SCREEN_H - 44) {
        drawMainScreen();
      }
      break;

    case SCR_CONFIRM:
      if (ty > SCREEN_H - 44) {
        bool yes = (tx < SCREEN_W / 2);
        if (confirmCb) confirmCb(yes);
        confirmCb = nullptr;
      }
      break;

    case SCR_WRITE_DONE:
      if (ty > SCREEN_H - 56) {
        if (tx < SCREEN_W / 2) {
          // Reprint: riscrive tag con le stesse impostazioni
          bool ok;
          if (writeMode == TAG_QIDI) {
            ok = writeTag();
          } else {
            ok = writeTagAnycubic();
          }
          delay(1500);
          drawWriteDone();  // torna alla schermata post-write
        } else {
          // Menu
          drawMainScreen();
        }
      }
      break;

    default:
      drawMainScreen();
      break;
  }
}

// ================================================================
//  AUTO MODE
// ================================================================
unsigned long lastAutoCheck = 0;

// Lettura auto robusta: rileva tag, poi fa 2 letture consecutive identiche
// Ritorna true solo se i dati sono confermati due volte
bool autoReadRobust() {
  // Scan veloce: aspetta fino a 300ms per un tag
  uint8_t uid[7]; uint8_t uidLen;
  if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 300)) return false;

  // Tag rilevato: tenta lettura dati
  uint8_t block1[16] = {0}, block2[16] = {0};
  bool ok1 = false, ok2 = false;

  if (uidLen == 4) {
    // MIFARE Classic
    ok1 = readTagInternal(uid, uidLen);
    if (ok1) memcpy(block1, tagBlock, 16);

    // Seconda lettura
    uint8_t uid2[7]; uint8_t uidLen2;
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid2, &uidLen2, 300)) return false;
    ok2 = readTagInternal(uid2, uidLen2);
    if (ok2) memcpy(block2, tagBlock, 16);

    if (!ok1 || !ok2) return false;
    if (memcmp(block1, block2, 16) != 0) {
      debugLog("Auto: mismatch, skip");
      return false;
    }
    // Dati confermati — ripristina tagBlock e info
    memcpy(tagBlock, block1, 16);
    memcpy(tagUID, uid, uidLen);
    tagUIDLen = uidLen;
    lastReadMode = TAG_QIDI;

  } else if (uidLen == 7) {
    // NTAG Anycubic
    ok1 = readTagAnycubicInternal(uid, uidLen);
    // Salva valori prima lettura
    String sku1 = aceReadSku, type1 = aceReadType;
    uint32_t col1 = aceReadColor;

    uint8_t uid2[7]; uint8_t uidLen2;
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid2, &uidLen2, 300)) return false;
    ok2 = readTagAnycubicInternal(uid2, uidLen2);

    if (!ok1 || !ok2) return false;
    // Confronta SKU e tipo: devono essere identici
    if (aceReadSku != sku1 || aceReadType != type1 || aceReadColor != col1) {
      debugLog("Auto: NTAG mismatch, skip");
      return false;
    }
    memcpy(tagUID, uid, uidLen);
    tagUIDLen = uidLen;
    lastReadMode = TAG_ANYCUBIC;

  } else {
    // Tag di tipo sconosciuto (UID != 4 né 7 byte)
    tagPresent = false;
    memcpy(tagUID, uid, uidLen);
    tagUIDLen = uidLen;
    return true;  // segnala tag rilevato ma non valido
  }

  // tagPresent è già stato impostato da readTagInternal / readTagAnycubicInternal
  return true;
}

// Helper: mostra schermata TAG SCONOSCIUTO in auto mode e aspetta rimozione/touch
void autoShowUnknown() {
  curScreen = SCR_READ_RESULT;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  setFontTitle();
  tft.drawString(L1(curLang == LANG_IT ? "TAG SCONOSCIUTO" : "UNKNOWN TAG"), SCREEN_W / 2, SCREEN_H / 2 - 20);
  setFontBody();
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(L1(curLang == LANG_IT ? "Dati non validi o tag vuoto" : "Invalid data or blank tag"), SCREEN_W / 2, SCREEN_H / 2 + 10);
  // In auto mode NON mostriamo il tasto Back — la schermata scompare con la rimozione del tag
  ledOff(); ledFlashErr(); ledOff();

  // Mantieni visualizzazione finché il tag è presente o touch
  unsigned long lastPresenceCheck = millis();
  while (true) {
    if (millis() - lastPresenceCheck >= 500) {
      lastPresenceCheck = millis();
      uint8_t reUid[7]; uint8_t reLen;
      bool still = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reLen, 200);
      if (!still) break;
    }
    if (touchIsTouched()) {
      int tx, ty;
      touchGetPoint(tx, ty);
      if (btnHit(btnStopAuto, tx, ty)) {
        autoMode = false;
        saveSettings();
      }
      while (touchIsTouched()) delay(10);
      break;
    }
    delay(20);
  }
}

void autoModeLoop() {
  if (!autoMode || !nfcPresent || nfcBusy) return;
  if (curScreen != SCR_MAIN) return;
  // Polling ogni 400ms (abbastanza reattivo, non sovraccarica il bus I2C)
  if (millis() - lastAutoCheck < 400) return;
  lastAutoCheck = millis();

  ledBlinkBlue();

  if (autoReadRobust()) {
    ledOff();
    if (!tagPresent) {
      // Tag rilevato ma dati vuoti/invalidi/tipo sconosciuto
      ledFlashErr(); ledOff();
      autoShowUnknown();
    } else {
      ledOff(); ledFlashOk(); ledOff();
      if (lastReadMode == TAG_ANYCUBIC) drawReadResultAnycubic(false);
      else drawReadResult(false);

      // Mantieni visualizzazione finché il tag è presente
      unsigned long lastPresenceCheck = millis();
      while (true) {
        // Check presenza ogni 500ms (veloce ma non stressante)
        if (millis() - lastPresenceCheck >= 500) {
          lastPresenceCheck = millis();
          uint8_t reUid[7]; uint8_t reLen;
          bool still = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reLen, 200);
          if (!still) break;  // Tag rimosso
        }
        // Touch per uscire
        if (touchIsTouched()) {
          int tx, ty;
          touchGetPoint(tx, ty);
          if (btnHit(btnStopAuto, tx, ty)) {
            autoMode = false;
            saveSettings();
          }
          while (touchIsTouched()) delay(10);
          break;
        }
        delay(20);
      }
    }
    drawMainScreen();
  } else {
    ledOff();
  }
}

// ================================================================
//  SPIFFS – CSV Load / Save
// ================================================================

// Parse one CSV field (handles quotes)
String csvField(const String& line, int& pos) {
  String field;
  if (pos >= (int)line.length()) return field;
  if (line[pos] == '"') {
    pos++;
    while (pos < (int)line.length() && line[pos] != '"') field += line[pos++];
    if (pos < (int)line.length()) pos++; // skip closing quote
    if (pos < (int)line.length() && line[pos] == ',') pos++; // skip comma
  } else {
    int comma = line.indexOf(',', pos);
    if (comma < 0) comma = line.length();
    field = line.substring(pos, comma);
    pos = comma + 1;
  }
  field.trim();
  return field;
}

void loadMaterialsCSV() {
  if (!SPIFFS.exists(CSV_PATH)) {
    Serial.println("CSV not found, using defaults");
    return;
  }
  File f = SPIFFS.open(CSV_PATH, "r");
  if (!f) return;

  // Reset dynamic lists
  aceMatCount = 0;
  // Reset QIDI matDB
  matCount = 0;

  String header = f.readStringUntil('\n'); // skip header

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int pos = 0;
    String type = csvField(line, pos);
    String mat  = csvField(line, pos);
    String sku  = csvField(line, pos);
    String emin = csvField(line, pos);
    String emax = csvField(line, pos);
    String bmin = csvField(line, pos);
    String bmax = csvField(line, pos);
    String notes= csvField(line, pos);

    type.toUpperCase();

    if (type == "QIDI" && matCount < MAX_MATERIALS) {
      strncpy(matDB[matCount].name, mat.c_str(), 23);
      matDB[matCount].name[23] = 0;
      strncpy(matDB[matCount].sku, sku.c_str(), 23);
      matDB[matCount].sku[23] = 0;
      matDB[matCount].extMin = emin.toInt();
      matDB[matCount].extMax = emax.toInt();
      matDB[matCount].bedMin = bmin.toInt();
      matDB[matCount].bedMax = bmax.toInt();
      // Estrai code da notes (es. "code=11")
      matDB[matCount].code = 0;
      int codePos = notes.indexOf("code=");
      if (codePos >= 0) matDB[matCount].code = notes.substring(codePos + 5).toInt();
      matDB[matCount].used = true;
      matCount++;
    }
    else if (type == "ANYCUBIC" && aceMatCount < MAX_ACE_MATERIALS) {
      strncpy(aceMatList[aceMatCount].name, mat.c_str(), 23);
      aceMatList[aceMatCount].name[23] = 0;
      strncpy(aceMatList[aceMatCount].sku, sku.c_str(), 23);
      aceMatList[aceMatCount].sku[23] = 0;
      aceMatList[aceMatCount].extMin = emin.toInt();
      aceMatList[aceMatCount].extMax = emax.toInt();
      aceMatList[aceMatCount].bedMin = bmin.toInt();
      aceMatList[aceMatCount].bedMax = bmax.toInt();
      aceMatList[aceMatCount].used = true;
      aceMatCount++;
    }
    // Generic lines in CSV are ignored (generic is a manufacturer, not a type)
  }
  f.close();
  Serial.printf("CSV loaded: QIDI=%d, Anycubic=%d\n", matCount, aceMatCount);
}

void saveMaterialsCSV() {
  File f = SPIFFS.open(CSV_PATH, "w");
  if (!f) { Serial.println("CSV write failed!"); return; }

  f.println("TagType,Material,SKU,ExtruderMin,ExtruderMax,BedMin,BedMax,Notes");

  for (int i = 0; i < matCount; i++) {
    if (!matDB[i].used) continue;
    f.printf("QIDI,%s,%s,%d,%d,%d,%d,code=%d\n",
      matDB[i].name, matDB[i].sku,
      matDB[i].extMin, matDB[i].extMax,
      matDB[i].bedMin, matDB[i].bedMax,
      matDB[i].code);
  }
  for (int i = 0; i < aceMatCount; i++) {
    if (!aceMatList[i].used) continue;
    f.printf("Anycubic,%s,%s,%d,%d,%d,%d,\n",
      aceMatList[i].name, aceMatList[i].sku,
      aceMatList[i].extMin, aceMatList[i].extMax,
      aceMatList[i].bedMin, aceMatList[i].bedMax);
  }
  f.close();
  Serial.println("CSV saved");
}

// Write default CSV to SPIFFS if not present
void createDefaultCSV() {
  if (SPIFFS.exists(CSV_PATH)) return;
  File f = SPIFFS.open(CSV_PATH, "w");
  if (!f) return;
  f.println("TagType,Material,SKU,ExtruderMin,ExtruderMax,BedMin,BedMax,Notes");
  // QIDI defaults (37 materiali ufficiali con codici)
  f.println("QIDI,PLA,,190,220,50,60,code=1");
  f.println("QIDI,PLA Matte,,190,220,50,60,code=2");
  f.println("QIDI,PLA Metal,,190,230,50,60,code=3");
  f.println("QIDI,PLA Silk,,200,230,50,60,code=4");
  f.println("QIDI,PLA-CF,,210,240,50,60,code=5");
  f.println("QIDI,PLA-Wood,,190,220,50,60,code=6");
  f.println("QIDI,PLA Basic,,190,220,50,60,code=7");
  f.println("QIDI,PLA Matte Basic,,190,220,50,60,code=8");
  f.println("QIDI,ABS,,220,250,90,100,code=11");
  f.println("QIDI,ABS-GF,,220,250,90,100,code=12");
  f.println("QIDI,ABS-Metal,,220,250,90,100,code=13");
  f.println("QIDI,ABS-Odorless,,220,250,90,100,code=14");
  f.println("QIDI,ASA,,240,260,90,100,code=18");
  f.println("QIDI,ASA-AERO,,240,260,90,100,code=19");
  f.println("QIDI,PA,,260,290,80,100,code=24");
  f.println("QIDI,PA-CF,,270,300,80,100,code=25");
  f.println("QIDI,UltraPA-CF25,,270,300,80,100,code=26");
  f.println("QIDI,PA12-CF,,270,300,80,100,code=27");
  f.println("QIDI,PAHT-CF,,280,320,90,110,code=30");
  f.println("QIDI,PAHT-GF,,280,320,90,110,code=31");
  f.println("QIDI,Support for PAHT,,280,320,90,110,code=32");
  f.println("QIDI,Support for PET/PA,,280,320,90,110,code=33");
  f.println("QIDI,PC/ABS-FR,,240,270,90,110,code=34");
  f.println("QIDI,PET-CF,,250,280,70,90,code=37");
  f.println("QIDI,PET-GF,,250,280,70,90,code=38");
  f.println("QIDI,PETG Basic,,220,250,60,80,code=39");
  f.println("QIDI,PETG Tough,,220,250,60,80,code=40");
  f.println("QIDI,PETG Rapido,,220,250,60,80,code=41");
  f.println("QIDI,PETG-CF,,220,250,60,80,code=42");
  f.println("QIDI,PETG-GF,,220,250,60,80,code=43");
  f.println("QIDI,PPS-CF,,300,340,100,120,code=44");
  f.println("QIDI,PETG Translucent,,220,250,60,80,code=45");
  f.println("QIDI,PPS-GF,,300,350,80,100,code=46");
  f.println("QIDI,PVA,,190,210,50,60,code=47");
  f.println("QIDI,TPU,,200,230,30,60,code=49");
  f.println("QIDI,TPU,,200,230,30,60,code=50");
  // Anycubic defaults
  f.println("Anycubic,PLA,AHPLBK-101,190,230,50,60,");
  f.println("Anycubic,PLA+,AHPLPBK-102,210,230,45,60,");
  f.println("Anycubic,PLA High Speed,AHHSBK-103,190,230,50,60,");
  f.println("Anycubic,PLA Matte,HYGBK-102,190,230,55,65,");
  f.println("Anycubic,PLA Silk,AHSCWH-102,200,230,55,65,");
  f.println("Anycubic,ABS,SHABBK-102,220,250,90,100,");
  f.println("Anycubic,ASA,SHASABK-101,240,280,90,100,");
  f.println("Anycubic,PETG,AHPEBK-101,230,250,70,90,");
  f.println("Anycubic,TPU,STPBK-101,210,230,25,60,");
  f.println("Anycubic,PLA Marble,AHPLMBK-101,200,230,50,60,");
  f.println("Anycubic,PLA SE,AHPLSEBK-101,190,230,55,65,");
  f.close();
  Serial.println("Default CSV created");
}

// ================================================================
//  WEB SERVER
// ================================================================

void handleWebRoot() {
  server.send_P(200, "text/html", WEBUI_HTML);
}

void handleGetMaterials() {
  String json = "{\"qidi\":[";
  for (int i = 0; i < matCount; i++) {
    if (i > 0) json += ",";
    json += "{\"code\":" + String(matDB[i].code) +
            ",\"mat\":\"" + String(matDB[i].name) +
            "\",\"sku\":\"" + String(matDB[i].sku) +
            "\",\"emin\":" + String(matDB[i].extMin) +
            ",\"emax\":" + String(matDB[i].extMax) +
            ",\"bmin\":" + String(matDB[i].bedMin) +
            ",\"bmax\":" + String(matDB[i].bedMax) + "}";
  }
  json += "],\"anycubic\":[";
  for (int i = 0; i < aceMatCount; i++) {
    if (i > 0) json += ",";
    json += "{\"code\":0" +
            String(",\"mat\":\"") + String(aceMatList[i].name) +
            "\",\"sku\":\"" + String(aceMatList[i].sku) +
            "\",\"emin\":" + String(aceMatList[i].extMin) +
            ",\"emax\":" + String(aceMatList[i].extMax) +
            ",\"bmin\":" + String(aceMatList[i].bedMin) +
            ",\"bmax\":" + String(aceMatList[i].bedMax) + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handlePostMaterials() {
  String body = server.arg("plain");

  // Simple JSON parse for our known structure
  // Reset counts
  matCount = 0;
  aceMatCount = 0;

  // Parse QIDI
  int qStart = body.indexOf("\"qidi\":[");
  if (qStart >= 0) {
    int qEnd = body.indexOf(']', qStart);
    String qSection = body.substring(qStart, qEnd);
    int pos = 0;
    while ((pos = qSection.indexOf("{\"mat\":", pos)) >= 0) {
      int objEnd = qSection.indexOf('}', pos);
      if (objEnd < 0) break;
      String obj = qSection.substring(pos, objEnd + 1);

      auto getStr = [&](const String& key) -> String {
        int k = obj.indexOf("\"" + key + "\":\"");
        if (k < 0) return "";
        k += key.length() + 4;
        int e = obj.indexOf('"', k);
        return (e > k) ? obj.substring(k, e) : "";
      };
      auto getInt = [&](const String& key) -> int {
        int k = obj.indexOf("\"" + key + "\":");
        if (k < 0) return 0;
        k += key.length() + 3;
        return obj.substring(k).toInt();
      };

      String name = getStr("mat");
      if (name.length() > 0 && matCount < MAX_MATERIALS) {
        strncpy(matDB[matCount].name, name.c_str(), 23);
        matDB[matCount].name[23] = 0;
        strncpy(matDB[matCount].sku, getStr("sku").c_str(), 23);
        matDB[matCount].sku[23] = 0;
        matDB[matCount].code = getInt("code");
        matDB[matCount].extMin = getInt("emin");
        matDB[matCount].extMax = getInt("emax");
        matDB[matCount].bedMin = getInt("bmin");
        matDB[matCount].bedMax = getInt("bmax");
        matDB[matCount].used = true;
        matCount++;
      }
      pos = objEnd + 1;
    }
  }

  // Parse Anycubic
  int aStart = body.indexOf("\"anycubic\":[");
  if (aStart >= 0) {
    int aEnd = body.indexOf(']', aStart);
    String aSection = body.substring(aStart, aEnd);
    int pos = 0;
    while ((pos = aSection.indexOf("{\"mat\":", pos)) >= 0) {
      int objEnd = aSection.indexOf('}', pos);
      if (objEnd < 0) break;
      String obj = aSection.substring(pos, objEnd + 1);

      auto getStr = [&](const String& key) -> String {
        int k = obj.indexOf("\"" + key + "\":\"");
        if (k < 0) return "";
        k += key.length() + 4;
        int e = obj.indexOf('"', k);
        return (e > k) ? obj.substring(k, e) : "";
      };
      auto getInt = [&](const String& key) -> int {
        int k = obj.indexOf("\"" + key + "\":");
        if (k < 0) return 0;
        k += key.length() + 3;
        return obj.substring(k).toInt();
      };

      String name = getStr("mat");
      if (name.length() > 0 && aceMatCount < MAX_ACE_MATERIALS) {
        strncpy(aceMatList[aceMatCount].name, name.c_str(), 23);
        aceMatList[aceMatCount].name[23] = 0;
        strncpy(aceMatList[aceMatCount].sku, getStr("sku").c_str(), 23);
        aceMatList[aceMatCount].sku[23] = 0;
        aceMatList[aceMatCount].extMin = getInt("emin");
        aceMatList[aceMatCount].extMax = getInt("emax");
        aceMatList[aceMatCount].bedMin = getInt("bmin");
        aceMatList[aceMatCount].bedMax = getInt("bmax");
        aceMatList[aceMatCount].used = true;
        aceMatCount++;
      }
      pos = objEnd + 1;
    }
  }

  // Parse Generic
  saveMaterialsCSV();
  // Also save QIDI materials to NVS for existing functionality
  saveMatDB();

  String resp = "{\"ok\":true,\"msg\":\"Saved: QIDI=" + String(matCount) +
    " Anycubic=" + String(aceMatCount) +
    " Anycubic=" + String(aceMatCount) + "\"}";
  server.send(200, "application/json", resp);
}

// ================================================================
//  WEB API – Settings GET/POST
// ================================================================
void handleGetSettings() {
  String json = "{";
  json += "\"lang\":" + String((int)curLang);
  json += ",\"autoMode\":" + String(autoMode ? "true" : "false");
  json += ",\"displayInvert\":" + String(displayInvert ? "true" : "false");
  json += ",\"backlight\":" + String(backlightLevel);
  json += ",\"ssTimeout\":" + String(screensaverTimeout / 1000);  // secondi
  json += ",\"rainbowEnabled\":" + String(rainbowEnabled ? "true" : "false");
  json += ",\"showDebugPanel\":" + String(showDebugPanel ? "true" : "false");
  json += ",\"firmware\":\"" + String(FW_VERSION) + "\"";
  json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"nfcPresent\":" + String(nfcPresent ? "true" : "false");
  json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
  json += ",\"uptime\":" + String(millis() / 1000);
  json += ",\"rssi\":" + String(WiFi.RSSI());
  // Manufacturers
  json += ",\"manufacturers\":[";
  for (int i = 0; i < mfgCount; i++) {
    if (i > 0) json += ",";
    json += "\"" + String(mfgDB[i].name) + "\"";
  }
  json += "]";
  json += "}";
  server.send(200, "application/json", json);
}

void handlePostSettings() {
  String body = server.arg("plain");

  // Parse lang
  int langIdx = body.indexOf("\"lang\":");
  if (langIdx >= 0) {
    int val = body.substring(langIdx + 7).toInt();
    if (val >= 0 && val < LANG_COUNT) curLang = (Lang)val;
  }

  // Parse autoMode
  autoMode = (body.indexOf("\"autoMode\":true") >= 0);

  // Parse displayInvert
  bool newInvert = (body.indexOf("\"displayInvert\":true") >= 0);
  if (newInvert != displayInvert) {
    displayInvert = newInvert;
    tft.invertDisplay(displayInvert);
  }

  // Parse backlight
  int blIdx = body.indexOf("\"backlight\":");
  if (blIdx >= 0) {
    int bl = body.substring(blIdx + 12).toInt();
    backlightLevel = constrain(bl, 0, 255);
    setBacklight(backlightLevel);
  }

  // Parse auto power-off timeout (secondi -> ms, 0 = disabilitato)
  int ssIdx = body.indexOf("\"ssTimeout\":");
  if (ssIdx >= 0) {
    int secs = body.substring(ssIdx + 12).toInt();
    screensaverTimeout = (unsigned long)constrain(secs, 0, 3600) * 1000UL;
  }

  // Parse rainbow title
  rainbowEnabled = (body.indexOf("\"rainbowEnabled\":true") >= 0);

  // Parse debug panel visibility
  showDebugPanel = (body.indexOf("\"showDebugPanel\":true") >= 0);

  // Parse manufacturers
  int mStart = body.indexOf("\"manufacturers\":[");
  if (mStart >= 0) {
    int mEnd = body.indexOf(']', mStart);
    String mSection = body.substring(mStart + 17, mEnd);
    mfgCount = 0;
    int pos = 0;
    while (pos < (int)mSection.length() && mfgCount < MAX_MANUFACTURERS) {
      int q1 = mSection.indexOf('"', pos);
      if (q1 < 0) break;
      int q2 = mSection.indexOf('"', q1 + 1);
      if (q2 < 0) break;
      String name = mSection.substring(q1 + 1, q2);
      if (name.length() > 0) {
        strncpy(mfgDB[mfgCount].name, name.c_str(), 23);
        mfgDB[mfgCount].name[23] = 0;
        mfgDB[mfgCount].used = true;
        mfgCount++;
      }
      pos = q2 + 1;
    }
    saveMfgDB();
  }

  saveSettings();
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Settings saved!\"}");

  // Redraw main screen to reflect changes
  if (curScreen == SCR_MAIN) drawMainScreen();
}

void handleResetWifi() {
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"WiFi reset. Rebooting...\"}");
  delay(500);
  WiFiManager wm;
  wm.resetSettings();
  delay(500);
  ESP.restart();
}

void handleResetMaterials() {
  initDefaultMfg();
  initDefaultMat();
  // Reset Anycubic materials from CSV
  aceMatCount = 0;
  if (SPIFFS.exists(CSV_PATH)) SPIFFS.remove(CSV_PATH);
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Materials reset to defaults!\"}");
}

void handleReboot() {
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Rebooting...\"}");
  delay(500);
  ESP.restart();
}

// ================================================================
//  QR CODE on TFT (uses ESP32 built-in esp_qrcode)
// ================================================================

// Globals for QR drawing callback
static int _qrOx, _qrOy, _qrMod;
static int _qrDrawnSize = 0;  // actual pixel size after draw

// Disegna il logo al centro di un'area
void drawLogoCentered(int cx, int cy) {
  int lx = cx - LOGO_W / 2;
  int ly = cy - LOGO_H / 2;
  // Bordo bianco attorno al logo
  tft.fillRect(lx - 2, ly - 2, LOGO_W + 4, LOGO_H + 4, TFT_WHITE);
  // Disegna pixel per pixel da PROGMEM
  for (int y = 0; y < LOGO_H; y++) {
    for (int x = 0; x < LOGO_W; x++) {
      uint16_t color = pgm_read_word(&logo_data[y * LOGO_W + x]);
      tft.drawPixel(lx + x, ly + y, color);
    }
  }
}

void _qrDrawCb(esp_qrcode_handle_t qrcode) {
  int size = esp_qrcode_get_size(qrcode);
  int qrPx = size * _qrMod;
  _qrDrawnSize = qrPx;
  // White background with margin
  tft.fillRect(_qrOx - 4, _qrOy - 4, qrPx + 8, qrPx + 8, TFT_WHITE);
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (esp_qrcode_get_module(qrcode, x, y)) {
        tft.fillRect(_qrOx + x * _qrMod, _qrOy + y * _qrMod,
                     _qrMod, _qrMod, TFT_BLACK);
      }
    }
  }
  // Logo al centro del QR
  int centerX = _qrOx + qrPx / 2;
  int centerY = _qrOy + qrPx / 2;
  drawLogoCentered(centerX, centerY);
}

void drawQRCode(const char* text, int ox, int oy, int moduleSize) {
  _qrOx = ox; _qrOy = oy; _qrMod = moduleSize;
  esp_qrcode_config_t cfg = {
    .display_func = _qrDrawCb,
    .max_qrcode_version = 10,
    .qrcode_ecc_level = ESP_QRCODE_ECC_HIGH,  // ECC HIGH per tollerare il logo al centro
  };
  esp_qrcode_generate(&cfg, text);
}

void showQRScreen(const char* title, const char* url, const char* subtitle) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(L1(title), SCREEN_W / 2, 18);

  // QR grande: moduleSize=5, ~33 moduli = ~165px centrato su 240x320
  int modSize = 5;
  int estPx = 33 * modSize;  // ~165px
  int ox = (SCREEN_W - estPx) / 2;
  int oy = (SCREEN_H - estPx) / 2 - 10;  // centrato verticalmente, un po' su per il subtitle
  drawQRCode(url, ox, oy, modSize);

  // Subtitle under QR
  int subY = oy + _qrDrawnSize + 16;
  setFontBody();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(L1(subtitle), SCREEN_W / 2, subY);
}

// ================================================================
//  LED arancione helper (active LOW)
// ================================================================
void ledOrange() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);
}
void ledYellow() {
  digitalWrite(LED_R, LOW);   // R ON
  digitalWrite(LED_G, LOW);   // G ON  — senza blue = giallo (identico a arancione ma duty diverso)
  digitalWrite(LED_B, HIGH);  // B OFF
}
void ledOff() {
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
}

// Lampeggio blu non bloccante — chiamare periodicamente durante lettura/scrittura
unsigned long _lastLedBlink = 0;
bool _ledBlinkState = false;
void ledBlinkBlue() {
  if (millis() - _lastLedBlink > 200) {
    _lastLedBlink = millis();
    _ledBlinkState = !_ledBlinkState;
    if (_ledBlinkState) {
      digitalWrite(LED_R, HIGH);  // R OFF
      digitalWrite(LED_G, HIGH);  // G OFF
      digitalWrite(LED_B, LOW);   // B ON  = BLU
    } else {
      ledOff();
    }
  }
}

// ================================================================
//  Web server – registra le route; server.begin() solo dopo WiFi connesso
// ================================================================
void startWebServer() {
  // Registra le route (sicuro anche senza WiFi)
  server.on("/", HTTP_GET, handleWebRoot);
  server.on("/api/materials", HTTP_GET, handleGetMaterials);
  server.on("/api/materials", HTTP_POST, handlePostMaterials);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handlePostSettings);
  server.on("/api/reset-wifi", HTTP_POST, handleResetWifi);
  server.on("/api/reset-materials", HTTP_POST, handleResetMaterials);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  // server.begin() viene chiamato solo dopo WiFi.connect in connectWifi()
  Serial.println("Web server routes registered (not started yet)");
}

// ================================================================
//  connectWifi – chiamato on-demand dal pulsante "Collega WiFi"
//  LED arancione durante connessione, spento alla fine.
// ================================================================
void connectWifi() {
  // LED arancione = connessione in corso
  ledOrange();

  // Mostra stato su TFT
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  setFontTitle();
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("WiFi...", SCREEN_W / 2, SCREEN_H / 2 - 10);
  setFontSmall();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting...", SCREEN_W / 2, SCREEN_H / 2 + 16);

  // Prima prova a connettersi con credenziali salvate (senza aprire AP)
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.println("Trying saved WiFi credentials...");

  bool connected = false;
  for (int attempt = 0; attempt < 3 && !connected; attempt++) {
    unsigned long start = millis();
    while (millis() - start < 5000) {
      if (WiFi.status() == WL_CONNECTED) { connected = true; break; }
      delay(100);
    }
    if (!connected) {
      Serial.printf("WiFi attempt %d failed, retrying...\n", attempt + 1);
      WiFi.disconnect();
      delay(500);
      WiFi.begin();
    }
  }

  if (connected) {
    wifiConnected = true;
    wifiEnabled   = true;
    ledOff();
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
    if (!webServerStarted) { server.begin(); webServerStarted = true; Serial.println("Web server started"); }
    String url = "http://" + WiFi.localIP().toString();
    showQRScreen("Connected!", url.c_str(), WiFi.localIP().toString().c_str());
    delay(2000);
  } else {
    // Nessuna credenziale salvata — apri config portal
    Serial.println("Saved credentials failed, starting config portal...");
    WiFi.disconnect(true);
    delay(200);

    WiFiManager wm;
    wm.setDebugOutput(true);
    wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
    wm.setConnectTimeout(15);

    wm.setAPCallback([](WiFiManager* wm) {
      Serial.println("Config portal started");
      String wifiQR = "WIFI:T:WPA;S:" + String(WIFI_PORTAL_SSID) + ";P:" + String(WIFI_PORTAL_PASS) + ";;";
      showQRScreen("Scan QR to setup WiFi", wifiQR.c_str(), WIFI_PORTAL_SSID);
      setFontSmall();
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("or connect to AP manually", SCREEN_W / 2, SCREEN_H - 16);
    });

    if (wm.autoConnect(WIFI_PORTAL_SSID, WIFI_PORTAL_PASS)) {
      wifiConnected = true;
      wifiEnabled   = true;
      ledOff();
      Serial.print("WiFi connected via portal! IP: ");
      Serial.println(WiFi.localIP());
      configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
      if (!webServerStarted) { server.begin(); webServerStarted = true; Serial.println("Web server started"); }
      String url = "http://" + WiFi.localIP().toString();
      showQRScreen("Connected!", url.c_str(), WiFi.localIP().toString().c_str());
      delay(2000);
    } else {
      wifiConnected = false;
      wifiEnabled   = false;
      ledOff();
      Serial.println("WiFi not connected");
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      setFontBody();
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WiFi: offline", SCREEN_W / 2, SCREEN_H / 2);
      delay(1500);
    }
  }
}

// ================================================================
//  disconnectWifi – scollegamento manuale
// ================================================================
void disconnectWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiConnected = false;
  wifiEnabled   = false;
  Serial.println("WiFi disconnected by user");
}

// ================================================================
//  setupWiFiAndServer – mantenuta per compatibilità (non più chiamata da setup)
// ================================================================
void setupWiFiAndServer() {
  startWebServer();
}



// ================================================================
//  SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
#ifdef DISPLAY_24
  Serial.println("HandRFID-Touch S024 (2.4\") starting...");
#else
  Serial.println("HandRFID-Touch S028 (2.8\") starting...");
#endif

  // Backlight
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 255);  // Full brightness

  // Display
  tft.init();
  tft.setAttribute(UTF8_SWITCH, false);
  tft.setRotation(0);  // Portrait
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  int cx = SCREEN_W / 2;

  // HandRFID grande e colorato
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  setFontTitle();
  tft.drawString("HandRFID", cx, 52);

  // Versione
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setFontBody();
  tft.drawString("V1.2", cx, 80);

  // Separatore
  tft.drawFastHLine(30, 100, SCREEN_W - 60, TFT_DARKGREY);

  // Crediti
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  setFontBody();
  tft.drawString("By", cx, 120);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Paolo Sambinello", cx, 142);
  tft.drawString("Davide Gatti", cx, 164);

  // Separatore
  tft.drawFastHLine(30, 184, SCREEN_W - 60, TFT_DARKGREY);

  // Sito web
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  setFontSmall();
  tft.drawString("www.survivalhacking.it", cx, 204);

  // Load settings from NVS
  loadSettings();
  tft.invertDisplay(displayInvert);
  setBacklight(backlightLevel);


#ifdef DISPLAY_24
  // S024: auto-detect CST820 (capacitive) first, fall back to XPT2046 (resistive)
  Wire1.begin(CTP_SDA, CTP_SCL);
  Wire1.beginTransmission(I2C_ADDR_CST820);
  if (Wire1.endTransmission() == 0) {
    touchType = TOUCH_CAPACITIVE;
    ctp.begin(Wire1);
    hasCalib = true;  // capacitive doesn't need calibration
    Serial.println("Touch: CST820 capacitive detected");
  } else {
    Wire1.end();
    touchType = TOUCH_RESISTIVE;
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TOUCH_CS);
    ts.begin(SPI);
    ts.setRotation(0);
    Serial.println("Touch: XPT2046 resistive detected");
  }
  setFontSmall();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(touchType == TOUCH_CAPACITIVE ? "Touch: Capacitive" : "Touch: Resistive",
                 SCREEN_W / 2, 242);
#else
  // S028: solo resistivo XPT2046 su bus SPI dedicato (VSPI)
  touchType = TOUCH_RESISTIVE;
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(0);
  Serial.println("Touch: XPT2046 resistive (dedicated SPI)");
  setFontSmall();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Touch: Resistive", SCREEN_W / 2, 242);
#endif
  delay(800);

  // I2C + NFC (GPIO 21/22)
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);  // 100kHz stabile
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (versiondata) {
    nfcPresent = true;
    nfc.SAMConfig();
    debugLogf("PN532 FW v%d ready", (versiondata >> 24) & 0xFF);
    Serial.print("PN532 found, firmware: ");
    Serial.println((versiondata >> 24) & 0xFF, DEC);
  } else {
    nfcPresent = false;
    debugLog("PN532 NOT FOUND!");
    Serial.println("PN532 not found!");
  }

  // SPIFFS + CSV materials
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
  } else {
    createDefaultCSV();
    loadMaterialsCSV();
  }

  // Load manufacturer DB from NVS
  loadMfgDB();
  // Cleanup: remove any stale manufacturer entries
  {
    bool dirty = false;
    for (int i = 0; i < mfgCount; i++) {
      String n = mfgDB[i].name;
      if (n != DEFAULT_MFG_1 && n != DEFAULT_MFG_2 && n != DEFAULT_MFG_3) {
        Serial.printf("Removing stale manufacturer: %s\n", mfgDB[i].name);
        for (int j = i; j < mfgCount - 1; j++) mfgDB[j] = mfgDB[j + 1];
        mfgCount--;
        i--;
        dirty = true;
      }
    }
    if (dirty) saveMfgDB();
  }
  // If CSV loaded QIDI materials, save to NVS too
  if (matCount > 0) saveMatDB();
  // Fallback: load from NVS if CSV had no QIDI entries
  if (matCount == 0) loadMatDB();

  // Web server (avviato sempre, WiFi è opzionale on-demand)
  startWebServer();

  // RGB LED off
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_R, HIGH);  // Active LOW
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);

  delay(1000);
  lastActivityTime = millis();

  // Auto-calibration: if resistive touch has no calibration, force it now
  if (touchType == TOUCH_RESISTIVE && !hasCalib) {
    runCalibration();
  }




  drawMainScreen();
}

// ================================================================
//  LOOP
// ================================================================
void loop() {
  // Touch handling with debounce
  if (touchIsTouched() && millis() - lastTouchTime > 250) {
    lastTouchTime = millis();
    lastActivityTime = millis();
    int tx, ty;
    touchGetPoint(tx, ty);
    // Se siamo in power-off (screensaver), qualsiasi touch riaccende e torna alla main
    if (curScreen == SCR_SCREENSAVER) {
      setBacklight(backlightLevel);
      while (touchIsTouched()) delay(10);
      drawMainScreen();
    } else {
      Serial.printf("TOUCH x=%d y=%d scr=%d scroll=%d\n", tx, ty, curScreen, scrollOffset);
      handleTouch(tx, ty);
      // Wait for release
      while (touchIsTouched()) delay(10);
    }
  }

  // Auto power-off dopo timeout di inattivita: display nero + backlight spenta (0 = mai)
  if (screensaverTimeout > 0 && curScreen != SCR_SCREENSAVER
      && millis() - lastActivityTime > screensaverTimeout) {
    curScreen = SCR_SCREENSAVER;
    tft.fillScreen(TFT_BLACK);
    setBacklight(0);
    ledOff();   // spegni LED per sicurezza
  }

  // Rainbow title animation (~10 fps, solo se abilitato)
  if (rainbowEnabled && curScreen == SCR_MAIN && millis() - lastRainbowUpdate > 100) {
    lastRainbowUpdate = millis();
    rainbowHueOffset = (rainbowHueOffset + 5) % 360;
    drawRainbowTitle();
  }

  // Auto mode
  autoModeLoop();

  // Web server (solo se avviato, cioè dopo connessione WiFi)
  if (webServerStarted) server.handleClient();

  delay(10);
}


