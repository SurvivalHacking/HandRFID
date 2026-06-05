// ============================================================
//  HandRFID – Multilingual strings (2 languages)
//  Index: 0=EN, 1=IT
// ============================================================
#pragma once
#include <Arduino.h>

enum Lang { LANG_EN=0, LANG_IT, LANG_COUNT };

static const char* langNames[LANG_COUNT] = {
  "English", "Italiano"
};

// ---- Main menu ----
static const char* strMainTitle[LANG_COUNT] = {
  "HandRFID", "HandRFID"
};
static const char* strReadTag[LANG_COUNT] = {
  "Read Tag", "Leggi tag"
};
static const char* strWriteTag[LANG_COUNT] = {
  "Write Tag", "Scrivi tag"
};
static const char* strAutoOn[LANG_COUNT] = {
  "Read Tag AUTO", "Leggi tag AUTO"
};
static const char* strAutoOff[LANG_COUNT] = {
  "Auto: OFF", "Auto: OFF"  // non usato nella UI, tenuto per compatibilita
};
static const char* strStopAuto[LANG_COUNT] = {
  "MAN", "MAN"
};
static const char* strReadyToRead[LANG_COUNT] = {
  "Ready to read", "Pronto per leggere"
};
static const char* strConfigure[LANG_COUNT] = {
  "Configure", "Configura"
};
static const char* strSelectMaterial[LANG_COUNT] = {
  "Select material", "Seleziona materiale"
};
static const char* strSelectColor[LANG_COUNT] = {
  "Select color", "Seleziona colore"
};

// ---- Tag mode selection ----
static const char* strSelectMode[LANG_COUNT] = {
  "Select type", "Seleziona yipo"
};
static const char* strSelectWeight[LANG_COUNT] = {
  "Select weight", "Seleziona peso"
};
static const char* strAnycubicTag[LANG_COUNT] = {
  "Anycubic (NTAG)", "Anycubic (NTAG)"
};
static const char* strQidiTag[LANG_COUNT] = {
  "QIDI (Classic)", "QIDI (Classic)"
};
static const char* strNtagDetected[LANG_COUNT] = {
  "NTAG detected (Anycubic)", "NTAG rilevato (Anycubic)"
};
static const char* strClassicDetected[LANG_COUNT] = {
  "Classic detected (QIDI)", "Classic rilevato (QIDI)"
};
static const char* strUnsupportedTag[LANG_COUNT] = {
  "Unsupported tag type!", "Tipo di tag non supportato!"
};
static const char* strAskTagType[LANG_COUNT] = {
  "Tag not recognized.\nSave as:", "Tag non riconosciuto.\nSalva come:"
};
static const char* strDiscardTag[LANG_COUNT] = {
  "Discard", "Scarta"
};

// ---- Status messages ----
static const char* strWaiting[LANG_COUNT] = {
  "Waiting for tag...", "In attesa del tag..."
};
static const char* strNfcBusy[LANG_COUNT] = {
  "NFC busy!", "NFC occupato!"
};
static const char* strNoTag[LANG_COUNT] = {
  "No tag found", "Nessun tag trovato"
};
static const char* strAuthFail[LANG_COUNT] = {
  "Auth failed!", "Auth fallita!"
};
static const char* strReadFail[LANG_COUNT] = {
  "Read failed!", "Lettura fallita!"
};
static const char* strReadOk[LANG_COUNT] = {
  "Read: tag detected", "Lettura: tag rilevato"
};
static const char* strWriteFail[LANG_COUNT] = {
  "Write failed!", "Scrittura fallita!"
};
static const char* strWriteOk[LANG_COUNT] = {
  "Write successful!", "Scrittura riuscita!"
};
static const char* strAutoDetected[LANG_COUNT] = {
  "Auto: tag detected", "Auto: tag rilevato"
};

// ---- Navigation ----
static const char* strBack[LANG_COUNT] = {
  "Back", "Indietro"
};
static const char* strTagInfo[LANG_COUNT] = {
  "Tag information", "Informazioni tag"
};
static const char* strColor[LANG_COUNT] = {
  "Color", "Colore"
};

// ---- Error ----
static const char* strNfcError[LANG_COUNT] = {
  "ERROR: PN532 not found!", "ERRORE: PN532 non trovato!"
};

// ---- Setup menu ----
static const char* strSetup[LANG_COUNT] = {
  "Setup", "Setup"
};
static const char* strLanguage[LANG_COUNT] = {
  "Language", "Lingua"
};
static const char* strSelectLang[LANG_COUNT] = {
  "Select language", "Seleziona lingua"
};
static const char* strCalibrate[LANG_COUNT] = {
  "Calibrate", "Calibra"
};
static const char* strCalibInstr[LANG_COUNT] = {
  "Press each point for at least 1 second!", "Premi ogni punto per almeno 1 secondo!"
};
static const char* strCalibSaved[LANG_COUNT] = {
  "Calibration saved", "Calibrazione salvata"
};
static const char* strCalibAbort[LANG_COUNT] = {
  "Calibration aborted", "Calibrazione annullata"
};
static const char* strFactoryRestored[LANG_COUNT] = {
  "Factory defaults restored", "Impostazioni ripristinate"
};

// ---- Keyboard ----
static const char* strEnterName[LANG_COUNT] = {
  "Enter name", "Inserisci nome"
};
static const char* strBksp[LANG_COUNT] = {
  "Bksp", "Canc"
};
static const char* strSpace[LANG_COUNT] = {
  "Space", "Spazio"
};
static const char* strCancel[LANG_COUNT] = {
  "Cancel", "Annulla"
};
static const char* strKeyboard[LANG_COUNT] = {
  "Keyboard", "Tastiera"
};

// ---- Manufacturer / material management ----
static const char* strNewMfg[LANG_COUNT] = {
  "New manufacturer", "Nuovo produttore"
};
static const char* strEditMfg[LANG_COUNT] = {
  "Edit manufacturer", "Modifica produttore"
};
static const char* strMfgList[LANG_COUNT] = {
  "Manufacturer list", "Lista produttori"
};
static const char* strNewMat[LANG_COUNT] = {
  "New material", "Nuovo materiale"
};
static const char* strEditMat[LANG_COUNT] = {
  "Edit material", "Modifica materiale"
};
static const char* strMatList[LANG_COUNT] = {
  "Material list", "Lista materiali"
};
static const char* strManufacturer[LANG_COUNT] = {
  "Manufacturer", "Produttore"
};
static const char* strMaterial[LANG_COUNT] = {
  "Material", "Materiale"
};
static const char* strCalibration[LANG_COUNT] = {
  "Calibration", "Calibrazione"
};

// ---- Dialogs ----
static const char* strSave[LANG_COUNT] = {
  "Save", "Salva"
};
static const char* strSelect[LANG_COUNT] = {
  "Select", "Seleziona"
};
static const char* strName[LANG_COUNT] = {
  "Name:", "Nome:"
};
static const char* strNumber[LANG_COUNT] = {
  "Number:", "Numero:"
};
static const char* strChooseFreeNum[LANG_COUNT] = {
  "Choose free number", "Scegli numero libero"
};
static const char* strChangeName[LANG_COUNT] = {
  "Change name", "Cambia nome"
};
static const char* strConfirm[LANG_COUNT] = {
  "Please confirm", "Confermare"
};
static const char* strYes[LANG_COUNT] = {
  "Yes", "Si"
};
static const char* strNo[LANG_COUNT] = {
  "No", "No"
};

// ---- Factory reset ----
static const char* strResetAll[LANG_COUNT] = {
  "Reset all settings\nto factory default\nsettings?",
  "Ripristinare tutte le\nimpostazioni di\nfabbrica?"
};
static const char* strFactoryDefault[LANG_COUNT] = {
  "Factory default", "Impost. fabbrica"
};
static const char* strResetWifi[LANG_COUNT] = {
  "Reset WiFi", "Reset WiFi"
};
static const char* strResetMaterials[LANG_COUNT] = {
  "Reset Materials", "Reset Materiali"
};
static const char* strResetWifiConfirm[LANG_COUNT] = {
  "Reset WiFi credentials?\nDevice will reboot\nand open config portal.",
  "Resettare credenziali WiFi?\nIl dispositivo si riavvia\ne apre il portale config."
};
static const char* strResetMatConfirm[LANG_COUNT] = {
  "WARNING: This will delete\nALL your materials\n(including custom ones)\nand restore only defaults.",
  "ATTENZIONE: cancella\nTUTTI i materiali\n(anche i personalizzati)\ne ripristina solo i default."
};
static const char* strResetMfgList[LANG_COUNT] = {
  "Reset manufacturer list",
  "Ripristinare elenco\nproduttori"
};
static const char* strResetMatList[LANG_COUNT] = {
  "Reset material list",
  "Ripristinare elenco\nmateriali"
};

static const char* strResetTag[LANG_COUNT] = {
  "Reset Tag (Bambu)", "Reset Tag (Bambu)"
};

// ---- WiFi on-demand ----
static const char* strConnectWifi[LANG_COUNT] = {
  "Connect WiFi", "Collega WiFi"
};
static const char* strDisconnectWifi[LANG_COUNT] = {
  "Disconnect WiFi", "Scollega WiFi"
};
static const char* strWifiDisconnected[LANG_COUNT] = {
  "WiFi disconnected", "WiFi disconnesso"
};

// ---- Tag result screen ----
static const char* strExtruder[LANG_COUNT] = {
  "Extruder", "Estrusore"
};
static const char* strBed[LANG_COUNT] = {
  "Bed", "Piatto"
};

// ---- Post-write screen ----
static const char* strReprint[LANG_COUNT] = {
  "Rewrite Tag", "Riscrivo Tag"
};
static const char* strMenu[LANG_COUNT] = {
  "Menu", "Menu"
};
static const char* strWriteDoneTitle[LANG_COUNT] = {
  "Tag written!", "Tag scritto!"
};
static const char* strWriteDoneAsk[LANG_COUNT] = {
  "Write another or\nreturn to menu?", "Scrivere un altro o\ntornare al menu?"
};

// ---- Notice (Klipper hints) ----
static const char* strNotice[LANG_COUNT] = {
  "Notice", "Avviso"
};
static const char* strPleaseNote[LANG_COUNT] = {
  "Please note!", "Attenzione!"
};
static const char* strKlipperHintMfg[LANG_COUNT] = {
  "For this manufacturer\nto be available\non the printer,\nedit officiall_filas_list.cfg in Klipper, save and restart",
  "Per rendere il produttore\ndisponibile sulla\nstampante,\nmodificare officiall_filas_list.cfg in Klipper, salvare e riavviare"
};
static const char* strKlipperHintMat[LANG_COUNT] = {
  "For this material\nto be available\non the printer,\nedit officiall_filas_list.cfg in Klipper, save and restart",
  "Per rendere il materiale\ndisponibile sulla\nstampante,\nmodificare officiall_filas_list.cfg in Klipper, salvare e riavviare"
};
static const char* strQidiLocked[LANG_COUNT] = {
  "QIDI/Anycubic/Generic locked", "QIDI/Anycubic/Generic bloccato"
};
static const char* strRead[LANG_COUNT] = {
  "Read", "Leggi"
};
static const char* strWrite[LANG_COUNT] = {
  "Write", "Scrivi"
};

// ---- Colors: definiti in config.h come colorPalette[] ----

// ---- Default QIDI materials ----
static const char* defaultMaterials[] = {
  "PLA", "PLA Matte","PLA Metal","PLA Silk","PLA-CF","PLA-Wood	6",
  "PLA Basic","PLA Matte Basic","ABS","ABS-GF","ABS-Metal","ABS-Odorless",
  "ASA","ASA-AERO","UltraPA","PA-CF","UltraPA-CF25","PA12-CF","PAHT-CF",
  "PAHT-GF","Support For PAHT","Support For PET/PA","PC/ABS-FR","PET-CF",
  "PET-GF","PETG Basic","PETG Tough","PETG Rapido","PETG-CF","PETG-GF",
  "PPS-CF","PETG Translucent","PVA","TPU-Aero","TPU"
};
#define NUM_DEFAULT_MATERIALS (sizeof(defaultMaterials)/sizeof(defaultMaterials[0]))
