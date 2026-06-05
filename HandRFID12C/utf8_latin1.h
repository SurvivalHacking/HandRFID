// ============================================================
//  utf8_latin1.h – compatibilità L1() per HandRFID
//
//  Con lang.h che usa escape \xNN dirette (Latin-1), i byte
//  sono già corretti in flash. L1() deve essere un no-op.
// ============================================================
#pragma once
#include <Arduino.h>

// Passa la stringa invariata — i byte \xNN sono già Latin-1
inline const char* utf8ToLatin1(const char* s) { return s; }

// Overload per Arduino String
inline const char* utf8ToLatin1(const String& s) { return s.c_str(); }

// Alias breve
#define L1(s) utf8ToLatin1(s)
