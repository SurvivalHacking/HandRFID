// HandRFID - Multilingual strings (3 languages)
// Index: 0=EN, 1=IT, 2=FR
//
// Stringhe FR: ogni byte accentato e' un token C isolato tra virgolette.
// Esempio: "Fran""\xE7""ais"  =>  il compilatore non puo' sbagliare.
// Compatibile con GFX font FreeSans*8b (range 0x20-0xFF).
#pragma once
#include <Arduino.h>

enum Lang { LANG_EN=0, LANG_IT, LANG_FR, LANG_COUNT };

static const char* langNames[LANG_COUNT] = {
  "English", "Italiano", "Fran""\xE7""ais"
};

// ---- Main menu ----
static const char* strMainTitle[LANG_COUNT] = { "HandRFID", "HandRFID", "HandRFID" };
static const char* strReadTag[LANG_COUNT] = { "Read Tag", "Leggi tag", "Lire tag" };
static const char* strWriteTag[LANG_COUNT] = { "Write Tag", "Scrivi tag", "\xC9""crire tag" };
static const char* strAutoOn[LANG_COUNT] = { "Read Tag AUTO", "Leggi tag AUTO", "Lire tag AUTO" };
static const char* strAutoOff[LANG_COUNT] = { "Auto: OFF", "Auto: OFF", "Auto: OFF" };
static const char* strStopAuto[LANG_COUNT] = { "MAN", "MAN", "MAN" };
static const char* strReadyToRead[LANG_COUNT] = { "Ready to read", "Pronto per leggere", "Pr""\xEA""t ""\xE0"" lire" };
static const char* strConfigure[LANG_COUNT] = { "Configure", "Configura", "Configurer" };
static const char* strSelectMaterial[LANG_COUNT] = { "Select material", "Seleziona materiale", "Choisir mat""\xE9""riau" };
static const char* strSelectColor[LANG_COUNT] = { "Select color", "Seleziona colore", "Choisir couleur" };

// ---- Tag mode selection ----
static const char* strSelectMode[LANG_COUNT] = { "Select type", "Seleziona tipo", "Choisir type" };
static const char* strSelectWeight[LANG_COUNT] = { "Select weight", "Seleziona peso", "Choisir poids" };
static const char* strAnycubicTag[LANG_COUNT] = { "Anycubic (NTAG)", "Anycubic (NTAG)", "Anycubic (NTAG)" };
static const char* strQidiTag[LANG_COUNT] = { "QIDI (Classic)", "QIDI (Classic)", "QIDI (Classic)" };
static const char* strNtagDetected[LANG_COUNT] = { "NTAG detected (Anycubic)", "NTAG rilevato (Anycubic)", "NTAG d""\xE9""tect""\xE9"" (Anycubic)" };
static const char* strClassicDetected[LANG_COUNT] = { "Classic detected (QIDI)", "Classic rilevato (QIDI)", "Classic d""\xE9""tect""\xE9"" (QIDI)" };
static const char* strUnsupportedTag[LANG_COUNT] = { "Unsupported tag type!", "Tipo di tag non supportato!", "Type de tag non support""\xE9""!" };
static const char* strAskTagType[LANG_COUNT] = { "Tag not recognized.\nSave as:", "Tag non riconosciuto.\nSalva come:", "Tag non reconnu.\nEnregistrer comme:" };
static const char* strDiscardTag[LANG_COUNT] = { "Discard", "Scarta", "Ignorer" };

// ---- Status messages ----
static const char* strWaiting[LANG_COUNT] = { "Waiting for tag...", "In attesa del tag...", "En attente du tag..." };
static const char* strNfcBusy[LANG_COUNT] = { "NFC busy!", "NFC occupato!", "NFC occup""\xE9""!" };
static const char* strNoTag[LANG_COUNT] = { "No tag found", "Nessun tag trovato", "Aucun tag trouv""\xE9" };
static const char* strAuthFail[LANG_COUNT] = { "Auth failed!", "Auth fallita!", "Auth ""\xE9""chou""\xE9""e!" };
static const char* strReadFail[LANG_COUNT] = { "Read failed!", "Lettura fallita!", "Lecture ""\xE9""chou""\xE9""e!" };
static const char* strReadOk[LANG_COUNT] = { "Read: tag detected", "Lettura: tag rilevato", "Lecture: tag d""\xE9""tect""\xE9" };
static const char* strWriteFail[LANG_COUNT] = { "Write failed!", "Scrittura fallita!", "\xC9""criture ""\xE9""chou""\xE9""e!" };
static const char* strWriteOk[LANG_COUNT] = { "Write successful!", "Scrittura riuscita!", "\xC9""criture r""\xE9""ussie!" };
static const char* strAutoDetected[LANG_COUNT] = { "Auto: tag detected", "Auto: tag rilevato", "Auto: tag d""\xE9""tect""\xE9" };

// ---- Navigation ----
static const char* strBack[LANG_COUNT] = { "Back", "Indietro", "Retour" };
static const char* strTagInfo[LANG_COUNT] = { "Tag information", "Informazioni tag", "Informations tag" };
static const char* strColor[LANG_COUNT] = { "Color", "Colore", "Couleur" };

// ---- Error ----
static const char* strNfcError[LANG_COUNT] = { "ERROR: PN532 not found!", "ERRORE: PN532 non trovato!", "ERREUR: PN532 introuvable!" };

// ---- Setup menu ----
static const char* strSetup[LANG_COUNT] = { "Setup", "Setup", "Config" };
static const char* strLanguage[LANG_COUNT] = { "Language", "Lingua", "Langue" };
static const char* strSelectLang[LANG_COUNT] = { "Select language", "Seleziona lingua", "Choisir langue" };
static const char* strCalibrate[LANG_COUNT] = { "Calibrate", "Calibra", "Calibrer" };
static const char* strCalibInstr[LANG_COUNT] = { "Press points for at least 1 sec.!", "Premi i punti per almeno 1 sec.!", "Appuyez sur chaque point 1 s min!" };
static const char* strCalibSaved[LANG_COUNT] = { "Calibration saved", "Calibrazione salvata", "Calibration sauvegard""\xE9""e" };
static const char* strCalibAbort[LANG_COUNT] = { "Calibration aborted", "Calibrazione annullata", "Calibration annul""\xE9""e" };
static const char* strFactoryRestored[LANG_COUNT] = { "Factory defaults restored", "Impostazioni ripristinate", "Param""\xE8""tres usine restaur""\xE9""s" };

// ---- Keyboard ----
static const char* strEnterName[LANG_COUNT] = { "Enter name", "Inserisci nome", "Entrer un nom" };
static const char* strBksp[LANG_COUNT] = { "Bksp", "Canc", "Suppr" };
static const char* strSpace[LANG_COUNT] = { "Space", "Spazio", "Espace" };
static const char* strCancel[LANG_COUNT] = { "Cancel", "Annulla", "Annuler" };
static const char* strKeyboard[LANG_COUNT] = { "Keyboard", "Tastiera", "Clavier" };

// ---- Manufacturer / material management ----
static const char* strNewMfg[LANG_COUNT] = { "New manufacturer", "Nuovo produttore", "Nouveau fabricant" };
static const char* strEditMfg[LANG_COUNT] = { "Edit manufacturer", "Modifica produttore", "Modifier fabricant" };
static const char* strMfgList[LANG_COUNT] = { "Manufacturer list", "Lista produttori", "Liste fabricants" };
static const char* strNewMat[LANG_COUNT] = { "New material", "Nuovo materiale", "Nouveau mat""\xE9""riau" };
static const char* strEditMat[LANG_COUNT] = { "Edit material", "Modifica materiale", "Modifier mat""\xE9""riau" };
static const char* strMatList[LANG_COUNT] = { "Material list", "Lista materiali", "Liste mat""\xE9""riaux" };
static const char* strManufacturer[LANG_COUNT] = { "Manufacturer", "Produttore", "Fabricant" };
static const char* strMaterial[LANG_COUNT] = { "Material", "Materiale", "Mat""\xE9""riau" };
static const char* strCalibration[LANG_COUNT] = { "Calibration", "Calibrazione", "Calibration" };

// ---- Dialogs ----
static const char* strSave[LANG_COUNT] = { "Save", "Salva", "Enregistrer" };
static const char* strSelect[LANG_COUNT] = { "Select", "Seleziona", "S""\xE9""lectionner" };
static const char* strName[LANG_COUNT] = { "Name:", "Nome:", "Nom:" };
static const char* strNumber[LANG_COUNT] = { "Number:", "Numero:", "Num""\xE9""ro:" };
static const char* strChooseFreeNum[LANG_COUNT] = { "Choose free number", "Scegli numero libero", "Choisir num""\xE9""ro libre" };
static const char* strChangeName[LANG_COUNT] = { "Change name", "Cambia nome", "Changer le nom" };
static const char* strConfirm[LANG_COUNT] = { "Please confirm", "Confermare", "Confirmer" };
static const char* strYes[LANG_COUNT] = { "Yes", "Si", "Oui" };
static const char* strNo[LANG_COUNT] = { "No", "No", "Non" };

// ---- Factory reset ----
static const char* strResetAll[LANG_COUNT] = {
  "Reset all settings\nto factory default\nsettings?",
  "Ripristinare tutte le\nimpostazioni di\nfabbrica?",
  "R""\xE9""initialiser tous\nles param""\xE8""tres\nusine?"
};
static const char* strFactoryDefault[LANG_COUNT] = { "Factory default", "Impost. fabbrica", "D""\xE9""fauts usine" };
static const char* strResetWifi[LANG_COUNT] = { "Reset WiFi", "Reset WiFi", "R""\xE9""init. WiFi" };
static const char* strResetMaterials[LANG_COUNT] = { "Reset Materials", "Reset Materiali", "R""\xE9""init. Mat""\xE9""riaux" };
static const char* strResetWifiConfirm[LANG_COUNT] = {
  "Reset WiFi credentials?\nDevice will reboot\nand open config portal.",
  "Resettare credenziali WiFi?\nIl dispositivo si riavvia\ne apre il portale config.",
  "R""\xE9""initialiser le WiFi?\nL'appareil red""\xE9""marrera\net ouvrira le portail config."
};
static const char* strResetMatConfirm[LANG_COUNT] = {
  "WARNING: This will delete\nALL your materials\n(including custom ones)\nand restore only defaults.",
  "ATTENZIONE: cancella\nTUTTI i materiali\n(anche i personalizzati)\ne ripristina solo i default.",
  "ATTENTION: supprime\nTOUS les mat""\xE9""riaux\n(y compris personnalis""\xE9""s)\net restaure les d""\xE9""fauts."
};
static const char* strResetMfgList[LANG_COUNT] = {
  "Reset manufacturer list", "Ripristinare elenco\nproduttori",
  "R""\xE9""initialiser liste\nfabricants"
};
static const char* strResetMatList[LANG_COUNT] = {
  "Reset material list", "Ripristinare elenco\nmateriali",
  "R""\xE9""initialiser liste\nmat""\xE9""riaux"
};
static const char* strResetTag[LANG_COUNT] = { "Reset Tag (Bambu)", "Reset Tag (Bambu)", "R""\xE9""init. Tag (Bambu)" };

// ---- WiFi on-demand ----
static const char* strConnectWifi[LANG_COUNT] = { "Connect WiFi", "Collega WiFi", "Connecter WiFi" };
static const char* strDisconnectWifi[LANG_COUNT] = { "Disconnect WiFi", "Scollega WiFi", "D""\xE9""connecter WiFi" };
static const char* strWifiDisconnected[LANG_COUNT] = { "WiFi disconnected", "WiFi disconnesso", "WiFi d""\xE9""connect""\xE9" };

// ---- Tag result screen ----
static const char* strExtruder[LANG_COUNT] = { "Extruder", "Estrusore", "Extrudeur" };
static const char* strBed[LANG_COUNT] = { "Bed", "Piatto", "Plateau" };

// ---- Post-write screen ----
static const char* strReprint[LANG_COUNT] = { "Rewrite Tag", "Riscrivo Tag", "R""\xE9""\xE9""crire Tag" };
static const char* strMenu[LANG_COUNT] = { "Menu", "Menu", "Menu" };
static const char* strWriteDoneTitle[LANG_COUNT] = { "Tag written!", "Tag scritto!", "Tag ""\xE9""crit!" };
static const char* strWriteDoneAsk[LANG_COUNT] = {
  "Write another or\nreturn to menu?", "Scrivere un altro o\ntornare al menu?",
  "\xC9""crire un autre ou\nretour au menu?"
};

// ---- Notice (Klipper hints) ----
static const char* strNotice[LANG_COUNT] = { "Notice", "Avviso", "Avis" };
static const char* strPleaseNote[LANG_COUNT] = { "Please note!", "Attenzione!", "Attention!" };
static const char* strKlipperHintMfg[LANG_COUNT] = {
  "For this manufacturer\nto be available\non the printer,\nedit officiall_filas_list.cfg in Klipper, save and restart",
  "Per rendere il produttore\ndisponibile sulla\nstampante,\nmodificare officiall_filas_list.cfg in Klipper, salvare e riavviare",
  "Pour ce fabricant\nsur l'imprimante,\nmodifier officiall_filas_list.cfg\ndans Klipper et red""\xE9""marrer"
};
static const char* strKlipperHintMat[LANG_COUNT] = {
  "For this material\nto be available\non the printer,\nedit officiall_filas_list.cfg in Klipper, save and restart",
  "Per rendere il materiale\ndisponibile sulla\nstampante,\nmodificare officiall_filas_list.cfg in Klipper, salvare e riavviare",
  "Pour ce mat""\xE9""riau\nsur l'imprimante,\nmodifier officiall_filas_list.cfg\ndans Klipper et red""\xE9""marrer"
};
static const char* strQidiLocked[LANG_COUNT] = { "QIDI/Anycubic/Generic locked", "QIDI/Anycubic/Generic bloccato", "QIDI/Anycubic/Generic verrouill""\xE9" };
static const char* strRead[LANG_COUNT] = { "Read", "Leggi", "Lire" };
static const char* strWrite[LANG_COUNT] = { "Write", "Scrivi", "\xC9""crire" };

// ---- Default QIDI materials ----
static const char* defaultMaterials[] = {
  "PLA", "PLA Matte","PLA Metal","PLA Silk","PLA-CF","PLA-Wood",
  "PLA Basic","PLA Matte Basic","ABS","ABS-GF","ABS-Metal","ABS-Odorless",
  "ASA","ASA-AERO","UltraPA","PA-CF","UltraPA-CF25","PA12-CF","PAHT-CF",
  "PAHT-GF","Support For PAHT","Support For PET/PA","PC/ABS-FR","PET-CF",
  "PET-GF","PETG Basic","PETG Tough","PETG Rapido","PETG-CF","PETG-GF",
  "PPS-CF","PETG Translucent","PVA","TPU-Aero","TPU"
};
#define NUM_DEFAULT_MATERIALS (sizeof(defaultMaterials)/sizeof(defaultMaterials[0]))
