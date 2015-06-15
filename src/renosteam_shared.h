#pragma once

#include "rehlds_api.h"

#ifdef WIN32
#define HIDDEN
#else
#define HIDDEN __attribute__((visibility("hidden")))
#endif

#define MAX_PLAYERS 32

extern void LCPrintf(bool critical, const char *fmt, ...);

extern cvar_t* pcv_mp_logecho;
extern char g_GameDir[256];

extern bool Renosteam_Utils_Init();
extern char* trimbuf(char *str);
