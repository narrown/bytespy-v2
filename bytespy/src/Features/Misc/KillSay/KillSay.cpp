#include "KillSay.h"
#include "../../../SDK/SDK.h"

void CKillSay::Event(IGameEvent* pEvent)
{
	if (!pEvent)
		return;

	const int localIndex = I::EngineClient->GetLocalPlayer();
	const int iAttackerUID = pEvent->GetInt("attacker");
	PlayerInfo_t localInfo{};
	if (!I::EngineClient->GetPlayerInfo(localIndex, &localInfo))
		return;
	const int iLocalUID = I::EngineClient->GetPlayerInfo().userID;

	if (iAttackerUID == iLocalUID)
	{

	}
}
