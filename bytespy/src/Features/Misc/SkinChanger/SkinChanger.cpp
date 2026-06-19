#include "SkinChanger.h"

#define Redirect(from, to) case from: { nWeaponIndex = to; break; }

bool CSkinChanger::HasSkinStateChanged()
{
	if (g_LastSkinState.Australium != Vars::Visuals::SkinChanger::Australium.Value) return true;
	if (g_LastSkinState.Festivized != Vars::Visuals::SkinChanger::Festivized.Value) return true;
	if (g_LastSkinState.UnusualEffect != Vars::Visuals::SkinChanger::Unusual.Value) return true;
	return false;
}

void CSkinChanger::UpdateSkinStateCache()
{
	g_LastSkinState.Australium = Vars::Visuals::SkinChanger::Australium.Value;
	g_LastSkinState.Festivized = Vars::Visuals::SkinChanger::Festivized.Value;
	g_LastSkinState.UnusualEffect = Vars::Visuals::SkinChanger::Unusual.Value;
}

void CSkinChanger::AddAttribute(CTFWeaponBase* weapon, int iIndex, float flValue)
{
	if (iIndex == -1)
		return;

	auto attributeList = reinterpret_cast<CAttributeList*>(
		reinterpret_cast<std::uintptr_t>(weapon) + 3512);
	if (!attributeList)
		return;

	attributeList->AddAttribute(iIndex, flValue);
}

void CSkinChanger::RedirectIndex(int& nWeaponIndex)
{
	switch (nWeaponIndex)
	{
		//Redirect(Misc_t_FryingPan, Misc_t_GoldFryingPan);
		Redirect(Soldier_m_RocketLauncher, Soldier_m_RocketLauncherR);
		Redirect(Scout_m_Scattergun, Scout_m_ScattergunR);
		Redirect(Pyro_m_FlameThrower, Pyro_m_FlameThrowerR);
		Redirect(Demoman_m_GrenadeLauncher, Demoman_m_GrenadeLauncherR);
		Redirect(Demoman_s_StickybombLauncher, Demoman_s_StickybombLauncherR);
		Redirect(Heavy_m_Minigun, Heavy_m_MinigunR);
		Redirect(Engi_t_Wrench, Engi_t_WrenchR);
		Redirect(Medic_s_MediGun, Medic_s_MediGunR);
		Redirect(Sniper_m_SniperRifle, Sniper_m_SniperRifleR);
		Redirect(Sniper_s_SMG, Sniper_s_SMGR);
		Redirect(Spy_t_Knife, Spy_t_KnifeR);
		Redirect(Spy_m_Revolver, Spy_m_RevolverR);
		Redirect(Engi_s_EngineersPistol, Engi_s_PistolR);
		Redirect(Soldier_s_SoldiersShotgun, Soldier_s_ShotgunR);
		Redirect(Pyro_s_PyrosShotgun, Pyro_s_ShotgunR);
		Redirect(Heavy_s_HeavysShotgun, Heavy_s_ShotgunR);
		Redirect(Engi_m_EngineersShotgun, Engi_m_ShotgunR);
		Redirect(Scout_t_Bat, Scout_t_BatR);
		Redirect(Soldier_t_Shovel, Soldier_t_ShovelR);
		Redirect(Pyro_t_FireAxe, Pyro_t_FireAxeR);
		Redirect(Demoman_t_Bottle, Demoman_t_BottleR);
		Redirect(Medic_t_Bonesaw, Medic_t_BonesawR);
		Redirect(Sniper_t_Kukri, Sniper_t_KukriR);
	default: break;
	}
}

void CSkinChanger::ApplyAttributes(CTFWeaponBase* Weapon)
{
	if (!Weapon)
		return;

	if (Vars::Visuals::SkinChanger::Australium.Value)
	{
		AddAttribute(Weapon, 2027, 1.0f);
		AddAttribute(Weapon, 2022, 1.0f);
		AddAttribute(Weapon, 542, 1.0f);
		AddAttribute(Weapon, 273, 1.0f);
	}

	if (Vars::Visuals::SkinChanger::Festivized.Value)
		AddAttribute(Weapon, 2053, 1.0f);

	// TODO: fixy
	if (Vars::Visuals::SkinChanger::Unusual.Value > 0)
	{
		float effectValue = (Vars::Visuals::SkinChanger::Unusual.Value == 1) ?
			4.0f :
			static_cast<float>(Vars::Visuals::SkinChanger::Unusual.Value + 699);

		AddAttribute(Weapon, 134, effectValue);   
		AddAttribute(Weapon, 370, effectValue);  
	}
}

void CSkinChanger::ApplySkins()
{
	const auto pLocal = H::Entities.GetLocal();
	const auto pWeapon = H::Entities.GetWeapon();

	if (!pWeapon || !pLocal)
		return;

	int& nWeaponIndex = pWeapon->m_iItemDefinitionIndex();
	RedirectIndex(nWeaponIndex);

	const auto& m_hMyWeapons = pLocal->m_hMyWeapons();
	for (int i = 0; m_hMyWeapons[i].IsValid(); i++)
	{
		auto Weapon = m_hMyWeapons[i].Get();
		if (!Weapon)
			continue;

		ApplyAttributes(Weapon);
	}

	if (HasSkinStateChanged())
	{
		I::ClientState->ForceFullUpdate();
		UpdateSkinStateCache();
	}
}

void CSkinChanger::FrameStageNotify()
{
	const auto pLocal = H::Entities.GetLocal();
	const auto pWeapon = H::Entities.GetWeapon();

	if (!pLocal || !pWeapon || !Vars::Visuals::SkinChanger::Enabled.Value)
		return;

	ApplySkins();
}