#include "precompiled.h"

CRHNSPlayer g_Players[MAX_PLAYERS];

void CRHNSPlayer::init(IGameClient* cl) {
	m_Id = cl->GetId();
	m_pClient = cl;
	clear();
}

void CRHNSPlayer::clear() {
	m_AuthKind = CA_UNKNOWN;
}

void CRHNSPlayer::authenticated(client_auth_kind authkind) {
	m_AuthKind = authkind;
}

const char* CRHNSPlayer::GetSteamId() {
	static char idstring[64];

	USERID_t* steamId = m_pClient->GetNetworkUserID();
	uint32 accId = (uint32)steamId->m_SteamID;

	switch (m_AuthKind) {
	case CA_STEAM:
		sprintf(idstring, "STEAM_%u:%u:%u", 0, accId & 1, accId >> 1);
		break;

	case CA_HLTV:
		sprintf(idstring, "HLTV");
		break;

	case CA_NOSTEAM:
		sprintf(idstring, "STEAM_ID_LAN");
		break;

	default:
		sprintf(idstring, "UNKNOWN");
		break;
	}

	return idstring;
}

void Renosteam_Players_Init() {
	for (int i = 0; i < g_RehldsSvs->GetMaxClients(); i++) {
		g_Players[i].init(g_RehldsSvs->GetClient(i));
	}
}

CRHNSPlayer* GetPlayerByUserIdPtr(USERID_t* pUserId) {
	for (int i = 0; i < g_RehldsSvs->GetMaxClients(); i++) {
		if (g_Players[i].getClient()->GetNetworkUserID() == pUserId)
			return &g_Players[i];
	}

	return NULL;
}

CRHNSPlayer* GetPlayerByClientPtr(IGameClient* cl) {
	return &g_Players[cl->GetId()];
}
