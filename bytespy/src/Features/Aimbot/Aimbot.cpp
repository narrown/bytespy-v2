#include "Aimbot.h"

#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotProjectile/AimbotProjectile.h"
#include "AimbotMelee/AimbotMelee.h"
#include "AutoDetonate/AutoDetonate.h"
#include "AutoAirblast/AutoAirblast.h"
#include "AutoHeal/AutoHeal.h"
#include "AutoRocketJump/AutoRocketJump.h"
#include "../Misc/Misc.h"
#include "../Visuals/Visuals.h"

//#define elseif else if // HAHA Love you lua... cough cough hicklehack roblox cough cough 

bool CAimbot::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (pLocal == nullptr || pWeapon == nullptr)
		return false;

	if (!pLocal->IsAlive())
		return false;

	if (pLocal->IsAGhost() || pLocal->IsTaunting())
		return false;

	bool stunned = pLocal->InCond(TF_COND_STUNNED) && (pLocal->m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE));
	if (stunned || pLocal->m_bFeignDeathReady())
		return false;

	if (pLocal->InCond(TF_COND_PHASE) || pLocal->InCond(TF_COND_STEALTHED) || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return false;

	auto dmgMultiplier = SDK::AttribHookValue(1, "mult_dmg", pWeapon);
	if (dmgMultiplier == 0)
		return false;

	if (I::EngineVGui->IsGameUIVisible())
		return false;

	return true;
}

void CAimbot::RunAimbot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSecondaryType)
{
	m_bRunningSecondary = bSecondaryType;
	EWeaponType weaponType = m_bRunningSecondary ? G::SecondaryWeaponType : G::PrimaryWeaponType;

	bool tempAttackState = G::CanPrimaryAttack;
	if (m_bRunningSecondary)
	{
		tempAttackState = G::CanPrimaryAttack;
		G::CanPrimaryAttack = G::CanSecondaryAttack;
	}

	if (weaponType == EWeaponType::HITSCAN)
	{
		F::AimbotHitscan.Run(pLocal, pWeapon, pCmd);
	}
	else if (weaponType == EWeaponType::PROJECTILE)
	{
		F::AimbotProjectile.Run(pLocal, pWeapon, pCmd);
	}
	else if (weaponType == EWeaponType::MELEE)
	{
		F::AimbotMelee.Run(pLocal, pWeapon, pCmd);
	}

	if (m_bRunningSecondary)
	{
		G::CanPrimaryAttack = tempAttackState;
	}
}

void CAimbot::RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pWeapon)
		return;

	if (Vars::Aimbot::General::AutoSecondarySwitch.Value && pWeapon->m_iClip1() < 1 && pWeapon->GetSlot() == SLOT_PRIMARY && pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE
		&& pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE_DECAP && pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE_CLASSIC
		&& pWeapon->m_iItemDefinitionIndex() != Engi_m_TheWidowmaker && pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheMachina && Vars::Aimbot::General::AimType.Value != Vars::Aimbot::General::AimTypeEnum::Off && !I::MatSystemSurface->IsCursorVisible())
	{
		I::EngineClient->ClientCmd_Unrestricted("slot2");
	}

	auto& proj = F::AimbotProjectile;

	if (proj.m_iLastTickCancel != 0)
	{
		pCmd->weaponselect = proj.m_iLastTickCancel;
		proj.m_iLastTickCancel = 0;
	}

	m_bRan = false;

	int curTick = I::GlobalVars->tickcount;

	if (std::abs(G::Target.second - curTick) > 32)
		G::Target = std::make_pair(0, 0);

	if (std::abs(G::AimPosition.second - curTick) > 32)
		G::AimPosition = { {}, 0 };

	if (pCmd->weaponselect != 0)
		return;

	F::AutoRocketJump.Run(pLocal, pWeapon, pCmd);
	F::AutoRocketJump.ScoutJump(pLocal, pWeapon, pCmd);
	F::AutoRocketJump.Detonator(pLocal, pWeapon, pCmd);

	if (!ShouldRun(pLocal, pWeapon))
		return;

	F::AutoDetonate.Run(pLocal, pWeapon, pCmd);
	F::AutoAirblast.Run(pLocal, pWeapon, pCmd);
	F::AutoHeal.Run(pLocal, pWeapon, pCmd);

	RunAimbot(pLocal, pWeapon, pCmd);
	RunAimbot(pLocal, pWeapon, pCmd, true);
}

void CAimbot::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pWeapon)
		return;

	this->RunMain(pLocal, pWeapon, pCmd);

	bool attackState = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);
	G::Attacking = attackState;
}
