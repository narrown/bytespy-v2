#include "TickHandler.h"

#include "../NetworkFix/NetworkFix.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"
#include "../Backtrack/Backtrack.h"

void CTickshiftHandler::Reset()
{
	m_bSpeedhack = m_bDoubletap = m_bRecharge = m_bWarp = false;
	m_iShiftedTicks = m_iShiftedGoal = 0;
}

Vec3* CTickshiftHandler::GetShootAngle()
{
	if (m_bShootAngle && I::ClientState->chokedcommands)
		return &m_vShootAngle;
	return nullptr;
}

void CTickshiftHandler::Recharge(CTFPlayer* pLocal)
{
	if (!m_bGoalReached)
		return;

	bool bPassive = m_bRecharge = false;

	static float flPassiveTime = 0.f;
	flPassiveTime = std::max(flPassiveTime - TICK_INTERVAL, -TICK_INTERVAL);
	if (Vars::CL_Move::Doubletap::PassiveRecharge.Value && 0.f >= flPassiveTime)
	{
		bPassive = true;
		flPassiveTime += 1.f / Vars::CL_Move::Doubletap::PassiveRecharge.Value;
	}

	if (m_iDeficit)
	{
		bPassive = true;
		m_iDeficit--, m_iShiftedTicks--;
	}

	if (!Vars::CL_Move::Doubletap::RechargeTicks.Value && !bPassive
		|| m_bDoubletap || m_bWarp || m_iShiftedTicks == m_iMaxShift || m_bSpeedhack)
		return;

	m_bRecharge = true;
	m_iShiftedGoal = m_iShiftedTicks + 1;
}

void CTickshiftHandler::Warp()
{
	if (!m_bGoalReached)
		return;

	m_bWarp = false;
	if (!Vars::CL_Move::Doubletap::Warp.Value
		|| !m_iShiftedTicks || m_bDoubletap || m_bRecharge || m_bSpeedhack)
		return;

	m_bWarp = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::CL_Move::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!m_bGoalReached)
		return;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack)
		return;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return;

	if (Vars::CL_Move::Doubletap::NoDTinair.Value && !pLocal->m_hGroundEntity())
		return;

	if (Vars::CL_Move::Doubletap::OnlyDTatlethal.Value) {
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL)) {
			auto pPlayer = pEntity->As<CTFPlayer>();
			Vec3 vDelta = pPlayer->m_vecOrigin() - pLocal->m_vecOrigin();
			const float realdistance = vDelta.Length();
			const int playerClass = pLocal->m_iClass();
			float lethalrange = 0.0f;
			switch (playerClass)
			{
			case TF_CLASS_SCOUT:
				lethalrange = 330.0f;
				break;
			case TF_CLASS_HEAVY:
				lethalrange = 250.0f;
				break;
			default:
				lethalrange = 200.0f;
				break;
			}
			if (realdistance <= lethalrange && !Vars::CritHack::ForceCrits.Value) {
				continue;
			}
			else {
				return;
			}
		}
	}

	bool bAttacking = G::PrimaryWeaponType == EWeaponType::MELEE ? pCmd->buttons & IN_ATTACK : G::Attacking;
	if (!G::CanPrimaryAttack && !G::Reloading || !bAttacking && !m_bDoubletap || F::AutoRocketJump.IsRunning())
		return;

	m_bDoubletap = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::CL_Move::Doubletap::TickLimit.Value + 1, 0);
	if (Vars::CL_Move::Doubletap::AntiWarp.Value)
		m_bAntiWarp = pLocal->m_hGroundEntity();
}

void CTickshiftHandler::Speedhack()
{
	m_bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!m_bSpeedhack)
		return;

	m_bDoubletap = m_bWarp = m_bRecharge = false;
}

void CTickshiftHandler::SaveShootPos(CTFPlayer* pLocal)
{
	if (!m_bDoubletap && !m_bWarp)
		m_vShootPos = pLocal->GetShootPos();
}
Vec3 CTickshiftHandler::GetShootPos()
{
	return m_vShootPos;
}

bool CTickshiftHandler::CanChoke()
{
	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	int iMaxTicks = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		iMaxTicks = std::min(iMaxTicks, 8);

	return I::ClientState->chokedcommands < 21 && F::Ticks.m_iShiftedTicks + I::ClientState->chokedcommands < iMaxTicks;
}

void CTickshiftHandler::FakeDuck(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!Vars::CL_Move::FakeDuck.Value)
		return;

	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;

	const int desired_ticks = Vars::CL_Move::FakeDuckTicks.Value;
	*pSendPacket = false;

	bool isducking = pLocal->m_bDucking();
	bool isducked = pLocal->m_vecViewOffset().z == 45;
	int choked = I::ClientState->chokedcommands;

	if (isducking && isducked && choked >= desired_ticks)
	{
		pCmd->buttons &= ~IN_DUCK;
		*pSendPacket = true;
	}
	else
	{
		pCmd->buttons |= IN_DUCK;
	}
}


void CTickshiftHandler::AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Vec3 vVelocity = {};
	static int iMaxTicks = 0;
	if (m_bAntiWarp)
	{
		int iTicks = GetTicks();
		iMaxTicks = std::max(iTicks + 1, iMaxTicks);

		Vec3 vAngles; Math::VectorAngles(vVelocity, vAngles);
		vAngles.y = pCmd->viewangles.y - vAngles.y;
		Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
		vForward *= vVelocity.Length2D();

		if (iTicks > std::max(iMaxTicks - 8, 3))
			pCmd->forwardmove = -vForward.x, pCmd->sidemove = -vForward.y;
		else if (iTicks > 3)
		{
			pCmd->forwardmove = pCmd->sidemove = 0.f;
			pCmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
		else
			pCmd->forwardmove = vForward.x, pCmd->sidemove = vForward.y;
	}
	else
	{
		vVelocity = pLocal->m_vecVelocity();
		iMaxTicks = 0;
	}

	/*
	static bool bSet = false;

	if (!m_bAntiWarp)
	{
		bSet = false;
		return;
	}

	if (G::Attacking != 1 && !bSet)
	{
		bSet = true;
		SDK::StopMovement(pLocal, pCmd);
	}
	else
		pCmd->forwardmove = pCmd->sidemove = 0.f;
	*/
}

bool CTickshiftHandler::ValidWeapon(CTFWeaponBase* pWeapon) // wow this is retarded
{
	if (!pWeapon)
		return false;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_GRAPPLINGHOOK:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
		return false;
	}

	return true;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = U::Hooks.m_mHooks["CL_Move"];

	m_iShiftedTicks--;
	if (m_iShiftedTicks < 0)
		return;
	if (m_iWait > 0)
		m_iWait--;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		m_iWait = 1;

	m_bGoalReached = bFinalTick && m_iShiftedTicks == m_iShiftedGoal;

	if (CL_Move)
		CL_Move->Call<void>(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{

	if (auto pWeapon = H::Entities.GetWeapon())
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			if (!G::CanSecondaryAttack)
				m_iWait = Vars::CL_Move::Doubletap::TickLimit.Value;
			break;
		default:
			if (!ValidWeapon(pWeapon))
				m_iWait = 2;
			else if (G::Attacking || !G::CanPrimaryAttack && !G::Reloading)
				m_iWait = Vars::CL_Move::Doubletap::TickLimit.Value;
		}
	}
	else
		m_iWait = 2;

	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	m_iMaxShift = sv_maxusrcmdprocessticks->GetInt();
	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		m_iMaxShift = std::min(m_iMaxShift, 8);
	m_iMaxShift -= std::max(24 - std::clamp(Vars::CL_Move::Doubletap::RechargeLimit.Value, 1, 24), F::AntiAim.YawOn() ? F::AntiAim.AntiAimTicks() : 0);

	while (m_iShiftedTicks > m_iMaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	m_iShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (m_iShiftedTicks <= 0)
	{
		m_iShiftedTicks = 0;
		return;
	}

	if (m_bSpeedhack)
	{
		m_iShiftedTicks = Vars::CL_Move::SpeedFactor.Value;
		m_iShiftedGoal = 0;
	}

	m_iShiftedGoal = std::clamp(m_iShiftedGoal, 0, m_iMaxShift);
	if (m_iShiftedTicks > m_iShiftedGoal) // normal use/doubletap/teleport
	{
		m_bShifting = m_bShifted = m_iShiftedTicks - 1 != m_iShiftedGoal;
		m_iShiftStart = m_iShiftedTicks;

#ifndef TICKBASE_DEBUG
		while (m_iShiftedTicks > m_iShiftedGoal)
			CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
		//CLMoveFunc(accumulated_extra_samples, bFinalTick);
#else
		if (Vars::Debug::Info.Value)
			SDK::Output("Pre loop", "", { 0, 255, 255, 255 });
		while (m_iShiftedTicks > m_iShiftedGoal)
		{
			if (Vars::Debug::Info.Value)
				SDK::Output("Pre move", "", { 0, 127, 255, 255 });
			CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
			if (Vars::Debug::Info.Value)
				SDK::Output("Post move", "\n", { 0, 127, 255, 255 });
		}
		if (Vars::Debug::Info.Value)
			SDK::Output("Post loop", "\n", { 0, 0, 255, 255 });
#endif

		m_bShifting = m_bAntiWarp = false;
		if (m_bWarp)
			m_iDeficit = 0;

		m_bDoubletap = m_bWarp = false;
	}
	else // else recharge, run once if we have any choked ticks
	{
		if (I::ClientState->chokedcommands)
			CLMoveFunc(accumulated_extra_samples, bFinalTick);
	}
}

void CTickshiftHandler::CLMoveManage(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	Recharge(pLocal);
	Warp();
	Speedhack();
}

void CTickshiftHandler::Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	CLMoveManage(pLocal);
	CLMove(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal)
		return;

	Doubletap(pLocal, pCmd);
	AntiWarp(pLocal, pCmd);
	//FakeDuck(pCmd, pSendPacket);
}

void CTickshiftHandler::ManagePacket(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!m_bDoubletap && !m_bWarp && !m_bSpeedhack)
		return;

	if ((m_bSpeedhack || m_bWarp) && G::Attacking == 1)
	{
		*pSendPacket = true;
		return;
	}

	*pSendPacket = m_iShiftedGoal == m_iShiftedTicks;
	if (I::ClientState->chokedcommands >= 21 // prevent overchoking
		|| m_iShiftedTicks == m_iShiftedGoal + Vars::CL_Move::Doubletap::TickLimit.Value - 1 && I::ClientState->chokedcommands) // unchoke if we are choking
		*pSendPacket = true;
}

int CTickshiftHandler::GetTicks(CTFWeaponBase* pWeapon)
{
	if (m_bDoubletap && m_iShiftedGoal < m_iShiftedTicks)
		return m_iShiftedTicks - m_iShiftedGoal;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack || F::AutoRocketJump.IsRunning())
		return 0;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return 0;
	
	return std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, m_iMaxShift);
}

int CTickshiftHandler::GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks)
{
	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return 1 + (iTicks - iDelay) / std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL);
}

int CTickshiftHandler::GetMinimumTicksNeeded(CTFWeaponBase* pWeapon)
{
	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return (GetShotsWithinPacket(pWeapon) - 1) * std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL) + iDelay;
}

void CTickshiftHandler::DrawDTBar(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Ticks) || !pLocal->IsAlive())
		return;

	const DragBox_t dtPos = Vars::Menu::TicksDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	if (!F::Ticks.m_bSpeedhack)
	{
		int iChoke = std::max(
			I::ClientState->chokedcommands
			- (F::AntiAim.YawOn() ? F::AntiAim.AntiAimTicks() : 0),
			0
		);
		int iTicks = std::clamp(F::Ticks.m_iShiftedTicks + iChoke, 0, F::Ticks.m_iMaxShift);
		float flTargetRatio = F::Ticks.m_iMaxShift > 0
			? float(iTicks) / F::Ticks.m_iMaxShift
			: 0.0f;

		static float flDisplayedRatio = 0.0f;
		constexpr float SMOOTH_SPEED = 15.0f;

		float delta = I::GlobalVars->frametime * SMOOTH_SPEED;
		delta = std::clamp(delta, 0.0f, 1.0f);
		flDisplayedRatio += (flTargetRatio - flDisplayedRatio) * delta;

		const int iWidth = H::Draw.Scale(150);
		const int iHeight = H::Draw.Scale(25);
		const int iLineHeight = H::Draw.Scale(2);
		const int iPadding = H::Draw.Scale(4);

		const int iPosX = dtPos.x - iWidth / 2;
		const int iPosY = dtPos.y;
		if (!Vars::Menu::RemoveBackground.Value)
		{
			H::Draw.FillRoundRect(iPosX, iPosY, iWidth, iHeight, H::Draw.Scale(4), { 0, 0, 0, 150 });
		}

		H::Draw.StringOutlined(
			fFont,
			iPosX + iPadding,
			iPosY + iPadding,
			Vars::Menu::Theme::Active.Value,
			{ 0, 0, 0, 150 },
			ALIGN_TOPLEFT,
			std::format("ticks {}/{}", iTicks, F::Ticks.m_iMaxShift).c_str()
		);

		const bool bCharged = (iTicks == F::Ticks.m_iMaxShift);
		if (bCharged || F::Ticks.m_iMaxShift || Vars::CL_Move::Fakelag::Fakelag.Value)
		{
			std::string sStatus;
			Color_t    clrStatus;

			if (Vars::CL_Move::Doubletap::NoDTinair.Value && !pLocal->m_hGroundEntity())
			{
				sStatus = "no dt (air)";

				float time = I::GlobalVars->curtime * 5.0f;
				float t = (std::sin(time) * 0.5f) + 0.5f;

				Color_t darkRed = { 150,  0,   0,   255 };
				Color_t lightRed = { 255, 80,  80,  255 };
				clrStatus = {
					static_cast<byte>(darkRed.r + t * (lightRed.r - darkRed.r)),
					static_cast<byte>(darkRed.g + t * (lightRed.g - darkRed.g)),
					static_cast<byte>(darkRed.b + t * (lightRed.b - darkRed.b)),
					255
				};
			}
			else if (!F::Ticks.m_iWait)
			{
				sStatus = "ready";
				clrStatus = Vars::Menu::Theme::Accent.Value;
			}
			else if (Vars::CL_Move::Fakelag::Fakelag.Value)
			{
				sStatus = "fakelag";

				float time = I::GlobalVars->curtime * 5.0f;
				float t = (std::sin(time) * 0.5f) + 0.5f;

				Color_t darkRed = { 150,  0,   0,   255 };
				Color_t lightRed = { 255, 80,  80,  255 };
				clrStatus = {
					static_cast<byte>(darkRed.r + t * (lightRed.r - darkRed.r)),
					static_cast<byte>(darkRed.g + t * (lightRed.g - darkRed.g)),
					static_cast<byte>(darkRed.b + t * (lightRed.b - darkRed.b)),
					255
				};
			}
			else if (!ValidWeapon(H::Entities.GetWeapon()))
			{
				sStatus = "weapon can't dt";

				float time = I::GlobalVars->curtime * 5.0f;
				float t = (std::sin(time) * 0.5f) + 0.5f;

				Color_t darkRed = { 150,  0,   0,   255 };
				Color_t lightRed = { 255, 80,  80,  255 };
				clrStatus = {
					static_cast<byte>(darkRed.r + t * (lightRed.r - darkRed.r)),
					static_cast<byte>(darkRed.g + t * (lightRed.g - darkRed.g)),
					static_cast<byte>(darkRed.b + t * (lightRed.b - darkRed.b)),
					255
				};
			}
			else
			{
				sStatus = bCharged ? "ready" : "waiting";
				clrStatus = bCharged
					? Vars::Menu::Theme::Accent.Value
					: Vars::Menu::Theme::Inactive.Value;
			}

			H::Draw.StringOutlined(
				fFont,
				iPosX + iWidth - iPadding,
				iPosY + iPadding,
				clrStatus,
				{ 0, 0, 0, 150 },
				ALIGN_TOPRIGHT,
				sStatus.c_str()
			);
		}

		const int iLineY = iPosY + iHeight - iLineHeight - iPadding;
		const int iLineWidth = iWidth - iPadding * 2;
		const int iFilledWidth = static_cast<int>(iLineWidth * flDisplayedRatio);

		// Always draw inner tick meter background
		H::Draw.FillRect(iPosX + iPadding, iLineY, iLineWidth, iLineHeight, { 45, 45, 45, 200 });

		if (iFilledWidth > 0)
		{
			H::Draw.FillRect(
				iPosX + iPadding,
				iLineY,
				iFilledWidth,
				iLineHeight,
				Vars::Menu::Theme::Accent.Value
			);
		}
	}
	else
	{
		H::Draw.StringOutlined(
			fFont,
			dtPos.x,
			dtPos.y,
			Vars::Menu::Theme::Active.Value,
			{ 0, 0, 0, 150 },
			ALIGN_TOP,
			std::format("speedhack x{}", Vars::CL_Move::SpeedFactor.Value).c_str()
		);
	}
}
