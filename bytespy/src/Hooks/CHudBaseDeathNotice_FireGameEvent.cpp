#include "../SDK/SDK.h"

#include "../SDK/Vars.h"

MAKE_SIGNATURE(CHudBaseDeathNotice_FireGameEvent, "client.dll", "4C 8B DC 49 89 53 ? 49 89 4B ? 41 55", 0x0);

MAKE_HOOK(CHudBaseDeathNotice_FireGameEvent, S::CHudBaseDeathNotice_FireGameEvent(), void,
	void* rcx, IGameEvent* event)
{
	int killer = I::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));

	if (killer == I::EngineClient->GetLocalPlayer())
	{
		int damagebits = event->GetInt("damagebits");
		int death_flags = event->GetInt("death_flags");

		switch (Vars::Visuals::Killfeed::Icon.Value)
		{
		case 1: event->SetString("weapon", "saw_kill"); break;
		case 2: event->SetString("weapon", "pumpkindeath"); break;
		case 3: event->SetString("weapon", "armageddon"); break;
		case 4: event->SetString("weapon", "vehicle"); break;
		case 5: event->SetString("weapon", "world"); break;
		case 6: event->SetString("weapon", "saxxy"); break;
		}

		switch (Vars::Visuals::Killfeed::Overlays.Value)
		{
		case 1:
			death_flags |= 0x0400;
			break;
		case 2:
			damagebits |= (1 << 20);
			break;
		}

		switch (Vars::Visuals::Killfeed::Domination.Value)
		{
		case 0: break;
		case 1: death_flags |= 1; break;
		case 2: death_flags |= 6; break;
		case 3: death_flags |= 5; break;
		}

		event->SetInt("damagebits", damagebits);
		event->SetInt("death_flags", death_flags);
	}

	CALL_ORIGINAL(rcx, event);
}