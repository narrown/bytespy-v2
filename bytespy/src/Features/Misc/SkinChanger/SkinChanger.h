#pragma once

#include "../../../SDK/SDK.h"
#include "../../../SDK/Vars.h"

#include <unordered_set>

struct SkinState
{
	bool Australium = false;
	bool Festivized = false;
	int UnusualEffect = 0;
};

static SkinState g_LastSkinState;

class CSkinChanger
{
	bool HasSkinStateChanged();
	void UpdateSkinStateCache();

	void AddAttribute(CTFWeaponBase* weapon, int iIndex, float flValue);
	void RedirectIndex(int& nWeaponIndex);
	void ApplyAttributes(CTFWeaponBase* weapon);
	void ApplySkins();
public:
	void FrameStageNotify();
};

ADD_FEATURE(CSkinChanger, SkinChanger)