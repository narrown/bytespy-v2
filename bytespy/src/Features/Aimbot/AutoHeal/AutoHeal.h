#pragma once
#include "../../../SDK/SDK.h"

#define GET_ENT_FROM_USER_ID(userid) I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(userid))

class CAutoHeal
{
	bool ActivateOnVoice(CTFPlayer* pLocal, CWeaponMedigun* pWeapon, CUserCmd* pCmd);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	void Reset();

	void ProcessPlayerHurt(IGameEvent* event);

	void PreventReload(CUserCmd* cmd);

	int m_SimResType = MEDIGUN_NUM_RESISTS;
	int m_GoalResType = MEDIGUN_BULLET_RESIST;
	bool m_IsChangingRes = false;
	bool m_ShouldPop = false;
	std::unordered_map<int, bool> m_mMedicCallers = {};
	bool m_IntermediateSwitchDone = false;
	int m_ResSwitchesRemaining = 0;
	int m_SwitchDelayTicks = 0;

};


ADD_FEATURE(CAutoHeal, AutoHeal) 