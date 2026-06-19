#pragma once
#define GET_ENT_FROM_USER_ID(userid) I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(userid))

class CKillSay
{
public:
	void Event(IGameEvent* pEvent)
};

inline CKillSay g_KillSay;
