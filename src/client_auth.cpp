#include "precompiled.h"

client_auth_context_t* g_CurrentAuthContext = NULL;

bool Renosteam_FinishClientAuth(IGameClient* cl)
{
	if (g_CurrentAuthContext == NULL) {
		LCPrintf(true, "WARNING: %s is called without active auth context\n", __FUNCTION__);
		return false;
	}

	// nosteam = bot
	if (g_CurrentAuthContext->nativeAuthFailed) {
		g_RehldsFuncs->SteamNotifyBotConnect(cl);
	}

	CRHNSPlayer* plr = GetPlayerByClientPtr(cl);

	if (g_CurrentAuthContext->hltv) {
		plr->authenticated(CA_HLTV);
	} else if (g_CurrentAuthContext->nativeAuthFailed) {
		plr->authenticated(CA_NOSTEAM);
	} else {
		plr->authenticated(CA_STEAM);
	}

	return true;
}

void SV_ConnectClient_hook(IRehldsHook_SV_ConnectClient* chain) {
	if (g_CurrentAuthContext != NULL) {
		LCPrintf(true, "WARNING: %s: recursive call\n", __FUNCTION__);
		chain->callNext();
		return;
	}
	
	client_auth_context_t authContext = client_auth_context_t();
	g_CurrentAuthContext = &authContext;
	chain->callNext();
	g_CurrentAuthContext = NULL;
}

int SV_CheckKeyInfo_hook(IRehldsHook_SV_CheckKeyInfo* chain, netadr_t *adr, char *protinfo, short unsigned int *port, int *pAuthProtocol, char *pszRaw, char *cdkey) {
	if (g_CurrentAuthContext == NULL) {
		LCPrintf(true, "WARNING: %s is called outside auth context\n",__FUNCTION__);
		return chain->callNext(adr, protinfo, port, pAuthProtocol, pszRaw, cdkey);
	}
	
	int authProto = atoi(g_engfuncs.pfnInfoKeyValue(protinfo, "prot"));
	if (authProto <= 0 || authProto > 4) {
		g_RehldsFuncs->RejectConnection(adr, "Invalid connection.\n");
		return 0;
	}

	strncpy(pszRaw, g_engfuncs.pfnInfoKeyValue(protinfo, "raw"), 32);
	pszRaw[33] = 0;

	strncpy(cdkey, g_engfuncs.pfnInfoKeyValue(protinfo, "cdkey"), 32);
	cdkey[33] = 0;

	*pAuthProtocol = authProto;
	*port = 27005;
	g_CurrentAuthContext->pAuthProto = pAuthProtocol;

	return 1;
}

int SV_FinishCertificateCheck_hook(IRehldsHook_SV_FinishCertificateCheck* chain, netadr_t *adr, int nAuthProtocol, char *szRawCertificate, char *userinfo) {
	if (g_CurrentAuthContext == NULL) {
		LCPrintf(true, "WARNING: %s is called outside auth context\n", __FUNCTION__);
		return chain->callNext(adr, nAuthProtocol, szRawCertificate, userinfo);
	}

	const char* hltv = g_engfuncs.pfnInfoKeyValue(userinfo, "*hltv");
	if (hltv && *hltv) {
		*g_CurrentAuthContext->pAuthProto = 3;
		g_CurrentAuthContext->hltv = true;

		sizebuf_t* pNetMessage = g_RehldsFuncs->GetNetMessage();
		int* pMsgReadCount = g_RehldsFuncs->GetMsgReadCount();

		//avoid "invalid steam certificate length" error
		if (*pMsgReadCount == pNetMessage->cursize) {
			pNetMessage->cursize += 1;
		}
	}
	
	return 1;
}

qboolean Steam_NotifyClientConnect_hook(IRehldsHook_Steam_NotifyClientConnect* chain, IGameClient *cl, const void *pvSteam2Key, unsigned int ucbSteam2Key) {
	if (g_CurrentAuthContext == NULL) {
		LCPrintf(true, "WARNING: %s is called outside auth context\n", __FUNCTION__);
		return chain->callNext(cl, pvSteam2Key, ucbSteam2Key);
	}

	qboolean authRes = chain->callNext(cl, pvSteam2Key, ucbSteam2Key);
	g_CurrentAuthContext->nativeAuthFailed = !authRes;

	return Renosteam_FinishClientAuth(cl) ? 1 : 0;
}

void Steam_NotifyClientDisconnect_hook(IRehldsHook_Steam_NotifyClientDisconnect* chain, IGameClient* cl) {
	chain->callNext(cl);
	GetPlayerByClientPtr(cl)->clear();
}

char *SV_GetIDString_hook(IRehldsHook_SV_GetIDString* chain, USERID_t *id) {
	static char idstring[64];

	CRHNSPlayer* plr = GetPlayerByUserIdPtr(id);
	if (plr) {
		strcpy(idstring, plr->GetSteamId());
	}
	else {
		uint32 accId = id->m_SteamID & 0xFFFFFFFF;
		switch (id->idtype) {
		case 1:
			if (accId == 0) {
				strcpy(idstring, "STEAM_ID_LAN");
			}
			else if (accId == 1) {
				strcpy(idstring, "STEAM_ID_PENDING");
			}
			else {
				sprintf(idstring, "STEAM_%u:%u:%u", 0, accId & 1, accId >> 1);
			}
			break;

		case 2:
			if (accId == 0) {
				strcpy(idstring, "VALVE_ID_LAN");
			}
			else if (accId == 1) {
				strcpy(idstring, "VALVE_ID_PENDING");
			}
			else {
				sprintf(idstring, "VALVE_%u:%u:%u", 0, accId & 1, accId >> 1);
			}
			break;


		case 3:
			strcpy(idstring, "HLTV");
			break;

		default:
			strcpy(idstring, "UNKNOWN");
			break;
		}
	}

	return idstring;
}

bool Auth_Init() {

	//
	// Install hooks
	//
	g_RehldsHookchains->SV_ConnectClient()->registerHook(&SV_ConnectClient_hook);
	g_RehldsHookchains->SV_CheckKeyInfo()->registerHook(&SV_CheckKeyInfo_hook);
	g_RehldsHookchains->SV_FinishCertificateCheck()->registerHook(&SV_FinishCertificateCheck_hook);
	g_RehldsHookchains->Steam_NotifyClientConnect()->registerHook(&Steam_NotifyClientConnect_hook);

	g_RehldsHookchains->SV_GetIDString()->registerHook(&SV_GetIDString_hook);

	g_RehldsHookchains->Steam_NotifyClientDisconnect()->registerHook(&Steam_NotifyClientDisconnect_hook);

	return true;
}
