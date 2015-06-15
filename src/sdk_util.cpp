// Selected portions of dlls/util.cpp from SDK 2.1.
// Functions copied from there as needed...
// And modified to avoid buffer overflows (argh).

/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/
#include "precompiled.h"

cvar_t* pcv_mp_logecho;
char g_GameDir[256];


//=========================================================
// UTIL_LogPrintf - Prints a logged message to console.
// Preceded by LOG: ( timestamp ) < message >
//=========================================================
void UTIL_LogPrintf( char *fmt, ... ) {
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf ( string, sizeof(string), fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	ALERT( at_logged, "%s", string );
}

void LCPrintf(bool critical, const char *fmt, ...) {
	va_list			argptr;
	static char		string[2048];
	static const char prefix[] = "[RENS]: ";

	va_start(argptr, fmt);
	vsnprintf((sizeof(prefix) - 1) + string, sizeof(string) - (sizeof(prefix) - 1), fmt, argptr);
	va_end(argptr);
	memcpy(string, prefix, (sizeof(prefix) - 1));

	bool bNeedWriteInConsole = true, bNeedWriteInLog = true;
	if (bNeedWriteInConsole && bNeedWriteInLog)
	{
		if (pcv_mp_logecho && pcv_mp_logecho->value != 0.0)
			bNeedWriteInConsole = false;
	}

	if (bNeedWriteInConsole)
		SERVER_PRINT(string);

	if (bNeedWriteInLog)
		ALERT(at_logged, string);
}

bool Renosteam_Utils_Init() {
	pcv_mp_logecho = g_engfuncs.pfnCVarGetPointer("mp_logecho");
	return true;
}

char* trimbuf(char *str) {
	char *ibuf = str;
	int i = 0;

	if (str == NULL) return NULL;
	for (ibuf = str; *ibuf && (byte)(*ibuf) < (byte)0x80 && isspace(*ibuf); ++ibuf)
		;

	i = strlen(ibuf);
	if (str != ibuf)
		memmove(str, ibuf, i);

	str[i] = 0;
	while (--i >= 0) {
		if (!isspace(ibuf[i]))
			break;
	}
	ibuf[++i] = 0;
	return str;
}
