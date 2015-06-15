#pragma once

#include "renosteam_shared.h"

struct client_auth_context_t {
	int protocol;
	bool nativeAuthFailed;
	netadr_t* adr;


	client_auth_context_t() {
		protocol = 0;
		nativeAuthFailed = false;
		adr = NULL;
	}
};



extern client_auth_context_t* g_CurrentAuthContext;

extern bool Auth_Init();
