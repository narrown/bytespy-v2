#include "Misc.h"

#include "../Backtrack/Backtrack.h"
#include "../TickHandler/TickHandler.h"
#include "../Players/PlayerUtils.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"

#define M_PI 3.14159265358

void CMisc::RunPre(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	CheatsBypass();
	PingReducer();
	WeaponSway();

	if (!pLocal)
		return;

	AntiAFK(pLocal, pCmd);
	InstantRespawnMVM(pLocal);

	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming() || pLocal->InCond(TF_COND_SHIELD_CHARGE) || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	AutoJump(pLocal, pCmd);
	BlockBot(pLocal, pCmd);
	EdgeJump(pLocal, pCmd);
	AutoJumpbug(pLocal, pCmd);
	AutoStrafe(pLocal, pCmd);
	AutoPeek(pCmd);
	MovementLock(pLocal, pCmd);
	BreakJump(pLocal, pCmd);
}

void CMisc::RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket)
{
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming() || pLocal->InCond(TF_COND_SHIELD_CHARGE))
		return;

	EdgeJump(pLocal, pCmd, true);
	TauntKartControl(pLocal, pCmd);
	FastMovement(pLocal, pCmd);
}

void CMisc::AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::Bunnyhop.Value)
		return;

	static bool bStaticJump = false, bStaticGrounded = false, bLastAttempted = false;
	const bool bLastJump = bStaticJump, bLastGrounded = bStaticGrounded;
	const bool bCurJump = bStaticJump = pCmd->buttons & IN_JUMP, bCurGrounded = bStaticGrounded = pLocal->m_hGroundEntity();
	const auto pWeapon = H::Entities.GetWeapon();
		if (!pWeapon)
			return;
	if (pWeapon && pWeapon->m_iItemDefinitionIndex() == Scout_m_BabyFacesBlaster)
	{
		if (!(pLocal->m_fFlags() & FL_ONGROUND))
		{
			pCmd->buttons &= ~IN_JUMP;
			return;
		}
	}

	if (bCurJump && bLastJump && (bCurGrounded ? !pLocal->IsDucking() : true))
	{
		if (!(bCurGrounded && !bLastGrounded))
			pCmd->buttons &= ~IN_JUMP;

		if (!(pCmd->buttons & IN_JUMP) && bCurGrounded && !bLastAttempted)
			pCmd->buttons |= IN_JUMP;
	}

	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
	{	// prevent more than 9 bhops occurring. if a server has this under that threshold they're retarded anyways
		static int iJumps = 0;
		if (bCurGrounded)
		{
			if (!bLastGrounded && pCmd->buttons & IN_JUMP)
				iJumps++;
			else
				iJumps = 0;

			if (iJumps > 9)
				pCmd->buttons &= ~IN_JUMP;
		}
	}
	bLastAttempted = pCmd->buttons & IN_JUMP;
}

void CMisc::AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoJumpbug.Value || !(pCmd->buttons & IN_DUCK) || pLocal->m_hGroundEntity() || pLocal->m_vecVelocity().z > -650.f)
		return;

	float flUnduckHeight = 20 * pLocal->m_flModelScale();
	float flTraceDistance = flUnduckHeight + 2;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	Vec3 vOrigin = pLocal->m_vecOrigin();
	SDK::TraceHull(vOrigin, vOrigin - Vec3(0, 0, flTraceDistance), pLocal->m_vecMins(), pLocal->m_vecMaxs(), pLocal->SolidMask(), &filter, &trace);
	if (!trace.DidHit() || trace.fraction * flTraceDistance < flUnduckHeight) // don't try if we aren't in range to unduck or are too low
		return;

	pCmd->buttons &= ~IN_DUCK;
	pCmd->buttons |= IN_JUMP;
}

void CMisc::AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoStrafe.Value || pLocal->m_hGroundEntity() || !(pLocal->m_afButtonLast() & IN_JUMP) && (pCmd->buttons & IN_JUMP))
		return;

	switch (Vars::Misc::Movement::AutoStrafe.Value)
	{
	case Vars::Misc::Movement::AutoStrafeEnum::Legit:
	{
		static auto cl_sidespeed = U::ConVars.FindVar("cl_sidespeed");
		const float flSideSpeed = cl_sidespeed ? cl_sidespeed->GetFloat() : 450.f;

		if (pCmd->mousedx)
		{
			pCmd->forwardmove = 0.f;
			pCmd->sidemove = pCmd->mousedx > 0 ? flSideSpeed : -flSideSpeed;
		}
		break;
	}
	case Vars::Misc::Movement::AutoStrafeEnum::Directional:
	{
		// credits: KGB
		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			break;

		float flForward = pCmd->forwardmove, flSide = pCmd->sidemove;

		Vec3 vForward, vRight; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
		vForward.z = vRight.z = 0.f;
		vForward.Normalize(), vRight.Normalize();

		Vec3 vWishDir = {}; Math::VectorAngles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f }, vWishDir);
		Vec3 vCurDir = {}; Math::VectorAngles(pLocal->m_vecVelocity(), vCurDir);
		float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
		if (fabsf(flDirDelta) > Vars::Misc::Movement::AutoStrafeMaxDelta.Value)
			break;

		float flTurnScale = Math::RemapVal(Vars::Misc::Movement::AutoStrafeTurnScale.Value, 0.f, 1.f, 0.9f, 1.f);
		float flRotation = DEG2RAD((flDirDelta > 0.f ? -90.f : 90.f) + flDirDelta * flTurnScale);
		float flCosRot = cosf(flRotation), flSinRot = sinf(flRotation);

		pCmd->forwardmove = flCosRot * flForward - flSinRot * flSide;
		pCmd->sidemove = flSinRot * flForward + flCosRot * flSide;
	}
	}
}

/*void CMisc::DrawPath()
{
	auto pLocal = H::Entities.GetLocal();
	static Vector navTarget = Vector(0, 0, 0);
	static bool hasTarget = false;
	Vector localOrigin = pLocal->GetAbsOrigin();
	if (Vars::Misc::Movement::NavbotDraw.Value)
	{
		float angle = SDK::StdRandomFloat(0.0f, 360.0f) * (M_PI / 180.0f);
		float distance = SDK::StdRandomFloat(500.0f, 1500.0f);
		navTarget = localOrigin + Vector(std::cos(angle) * distance, std::sin(angle) * distance, localOrigin.z);

		Color_t Pathcolor = Vars::Misc::Movement::NavbotDrawColor.Value;
		Vec3     vLocalOrigin = pLocal->GetAbsOrigin();
		Vec3     screenLocal, screenNav;

		if (SDK::W2S(vLocalOrigin, screenLocal, true) &&
			SDK::W2S(navTarget, screenNav, true))
		{
			// cast float→int for the line API
			int x1 = static_cast<int>(screenLocal.x);
			int y1 = static_cast<int>(screenLocal.y);
			int x2 = static_cast<int>(screenNav.x);
			int y2 = static_cast<int>(screenNav.y);

			// draw it
			H::Draw.Line(x1, y1, x2, y2, Pathcolor);
		}
	}
}*/

void CMisc::BlockBot(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (Vars::Misc::Movement::NavBot.Value)
	{
		if (!pLocal || !pLocal->IsAlive())
			return;

		static Vector navTarget = Vector(0, 0, 0);
		static bool hasTarget = false;
		Vector localOrigin = pLocal->GetAbsOrigin();

		// Generate new target if needed
		if (!hasTarget || (localOrigin - navTarget).Length() < 50.0f)
		{
			float angle = SDK::StdRandomFloat(0.0f, 360.0f) * (M_PI / 180.0f);
			float distance = SDK::StdRandomFloat(500.0f, 1500.0f);
			navTarget = localOrigin + Vector(std::cos(angle) * distance, std::sin(angle) * distance, localOrigin.z);
			hasTarget = true;
		}

		// Wall avoidance logic
		if (Vars::Misc::Movement::NavBotAvoidWalls.Value)
		{
			trace_t trace;
			Ray_t ray;
			ray.Init(localOrigin, navTarget);
			CTraceFilterWorldAndPropsOnly filter;
			I::EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.fraction < 1.0f)
			{
				// Adjust target along wall tangent
				Vector tangent = trace.plane.normal.Cross(Vector(0, 0, 1));
				tangent.Normalize();
				navTarget = trace.endpos + tangent * 100.0f;
			}
		}

		/*if (Vars::Misc::Movement::NavbotDraw.Value)
		{
			DrawPath();
		}*/

		SDK::WalkTo(pCmd, pLocal, localOrigin, navTarget, 1.0f);
		return; // Skip BlockBot when NavBot is active
	}

	if (!Vars::Misc::Movement::BlockBot.Value)
		return;

	// Original BlockBot implementation
	const auto& teammates = H::Entities.GetGroup(EGroupType::PLAYERS_TEAMMATES);
	Vector localOrigin = pLocal->GetAbsOrigin();

	CTFPlayer* closestTeammate = nullptr;
	float closestDistance = FLT_MAX;

	for (auto baseEnt : teammates)
	{
		if (!baseEnt || baseEnt == pLocal)
			continue;

		auto pTeammate = reinterpret_cast<CTFPlayer*>(baseEnt);
		if (!pTeammate->IsAlive())
			continue;

		if (Vars::Misc::Movement::BlockBotSnipersOnly.Value && pTeammate->m_iClass() != TF_CLASS_SNIPER)
			continue;

		float dist = (pTeammate->GetAbsOrigin() - localOrigin).Length();
		if (dist < closestDistance)
		{
			closestDistance = dist;
			closestTeammate = pTeammate;
		}
	}

	if (!closestTeammate)
		return;

	float yawRad = DEG2RAD(closestTeammate->m_angEyeAnglesY());
	Vector forward(std::cos(yawRad), std::sin(yawRad), 0.0f);
	Vector targetPos = closestTeammate->GetAbsOrigin() + forward * Vars::Misc::Movement::BlockBotOffset.Value;

	SDK::WalkTo(pCmd, pLocal, localOrigin, targetPos, 1.0f);
}

void CMisc::jewishkids()
{
	if (!Vars::CL_Move::AutoPeek.Value || !m_bPeekPlaced)
		return;

	constexpr int segments = 32;
	constexpr float radius = 20.f;

	Color_t outlineColor = Vars::CL_Move::AutoPeekColor.Value;
	Color_t fillColor = outlineColor;
	fillColor.a = 100; 

	std::vector<Vertex_t> vertices;
	vertices.reserve(segments);

	for (int i = 0; i < segments; ++i)
	{
		float angle = DEG2RAD(360.f / segments * i);
		Vec3 point3D = m_vPeekReturnPos + Vec3(std::cos(angle) * radius, std::sin(angle) * radius, 0.f);

		Vec3 screenPos;
		if (!SDK::W2S(point3D, screenPos, false))  
			return; 

		Vertex_t vtx;
		vtx.m_Position.x = screenPos.x;
		vtx.m_Position.y = screenPos.y;

		vertices.push_back(vtx);
	}

	H::Draw.FillPolygon(vertices, fillColor);

	for (int i = 0; i < segments; ++i)
	{
		Vec3 p1 = Vec3(vertices[i].m_Position.x, vertices[i].m_Position.y, 0);
		Vec3 p2 = Vec3(vertices[(i + 1) % segments].m_Position.x, vertices[(i + 1) % segments].m_Position.y, 0);

		H::Draw.Line(static_cast<int>(p1.x), static_cast<int>(p1.y),
			static_cast<int>(p2.x), static_cast<int>(p2.y),
			outlineColor);
	}
}

void CMisc::AutoPeek(CUserCmd* pCmd)
{
	const auto pLocal = H::Entities.GetLocal();
	static bool bReturning = false;
	if (Vars::CL_Move::AutoPeek.Value)
	{
		const Vec3 vLocalPos = pLocal->m_vecOrigin();

		if (G::Attacking && m_bPeekPlaced)
			bReturning = true;
		if (!bReturning && !pLocal->m_hGroundEntity())
			m_bPeekPlaced = false;

		if (!m_bPeekPlaced)
		{
			m_vPeekReturnPos = vLocalPos;
			m_bPeekPlaced = true;
		}
		else
		{
			static Timer tTimer = {};
			if (tTimer.Run(0.7f))
				jewishkids();
		}

		if (bReturning)
		{
			if (vLocalPos.DistTo(m_vPeekReturnPos) < 8.f)
			{
				bReturning = false;
				return;
			}

			SDK::WalkTo(pCmd, pLocal, m_vPeekReturnPos);
			pCmd->buttons &= ~IN_JUMP;
		}
	}
	else
		m_bPeekPlaced = bReturning = false;
}

void CMisc::MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static bool bLock = false;

	if (!Vars::Misc::Movement::MovementLock.Value)
	{
		bLock = false;
		return;
	}

	static Vec3 vMove = {}, vView = {};
	if (!bLock)
	{
		bLock = true;
		vMove = { pCmd->forwardmove, pCmd->sidemove, pCmd->upmove };
		vView = pCmd->viewangles;
	}

	pCmd->forwardmove = vMove.x, pCmd->sidemove = vMove.y, pCmd->upmove = vMove.z;
	SDK::FixMovement(pCmd, vView, pCmd->viewangles);
}

void CMisc::BreakJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::BreakJump.Value || F::AutoRocketJump.IsRunning())
		return;

	static bool bStaticJump = false;
	const bool bLastJump = bStaticJump;
	const bool bCurrJump = bStaticJump = pCmd->buttons & IN_JUMP;

	static int iTickSinceGrounded = -1;
	if (pLocal->m_hGroundEntity().Get())
		iTickSinceGrounded = -1;
	iTickSinceGrounded++;

	switch (iTickSinceGrounded)
	{
	case 0:
		if (bLastJump || !bCurrJump || pLocal->IsDucking())
			return;
		break;
	case 1:
		break;
	default:
		return;
	}

	pCmd->buttons |= IN_DUCK;
}

void CMisc::AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Timer tTimer = {};

	static auto mp_idledealmethod = U::ConVars.FindVar("mp_idledealmethod");
	static auto mp_idlemaxtime = U::ConVars.FindVar("mp_idlemaxtime");
	const int iIdleMethod = mp_idledealmethod ? mp_idledealmethod->GetInt() : 1;
	const float flMaxIdleTime = mp_idlemaxtime ? mp_idlemaxtime->GetFloat() : 3.f;

	if (pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK) || !pLocal->IsAlive())
		tTimer.Update();
	else if (Vars::Misc::Automation::AntiAFK.Value && iIdleMethod && tTimer.Check(flMaxIdleTime * 60.f - 10.f)) // trigger 10 seconds before kick
		pCmd->buttons |= I::GlobalVars->tickcount % 2 ? IN_FORWARD : IN_BACK;
}

void CMisc::InstantRespawnMVM(CTFPlayer* pLocal)
{
	if (Vars::Misc::MannVsMachine::InstantRespawn.Value && I::EngineClient->IsInGame() && !pLocal->IsAlive())
	{
		KeyValues* kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::EngineClient->ServerCmdKeyValues(kv);
	}
}

void CMisc::CheatsBypass()
{
	static bool bCheatSet = false;
	static auto sv_cheats = U::ConVars.FindVar("sv_cheats");
	if (sv_cheats)
	{
		if (Vars::Misc::Exploits::CheatsBypass.Value)
		{
			sv_cheats->m_nValue = 1;
			bCheatSet = true;
		}
		else if (bCheatSet)
		{
			sv_cheats->m_nValue = 0;
			bCheatSet = false;
		}
	}
}

void CMisc::PingReducer()
{
	static Timer tTimer = {};
	if (!tTimer.Run(0.1f))
		return;

	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	static auto cl_cmdrate = U::ConVars.FindVar("cl_cmdrate");
	const int iCmdRate = cl_cmdrate ? cl_cmdrate->GetInt() : 66;

	// force highest cl_updaterate command possible
	static auto sv_maxupdaterate = U::ConVars.FindVar("sv_maxupdaterate");
	const int iMaxUpdateRate = sv_maxupdaterate ? sv_maxupdaterate->GetInt() : 66;

	const int iTarget = Vars::Misc::Exploits::PingReducer.Value ? Vars::Misc::Exploits::PingTarget.Value : iCmdRate;
	if (m_iWishCmdrate != iTarget)
	{
		NET_SetConVar cmd("cl_cmdrate", std::to_string(m_iWishCmdrate = iTarget).c_str());
		pNetChan->SendNetMsg(cmd);
	}

	if (m_iWishUpdaterate != iMaxUpdateRate)
	{
		NET_SetConVar cmd("cl_updaterate", std::to_string(m_iWishUpdaterate = iMaxUpdateRate).c_str());
		pNetChan->SendNetMsg(cmd);
	}
}

void CMisc::WeaponSway()
{
	static auto cl_wpn_sway_interp = U::ConVars.FindVar("cl_wpn_sway_interp");
	static auto cl_wpn_sway_scale = U::ConVars.FindVar("cl_wpn_sway_scale");

	bool bSway = Vars::Visuals::Viewmodel::SwayInterp.Value || Vars::Visuals::Viewmodel::SwayScale.Value;
	if (cl_wpn_sway_interp)
		cl_wpn_sway_interp->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayInterp.Value : 0.f);
	if (cl_wpn_sway_scale)
		cl_wpn_sway_scale->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayScale.Value : 0.f);
}



void CMisc::TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	// Handle Taunt Slide
	if (Vars::Misc::Automation::TauntControl.Value && pLocal->IsTaunting() && pLocal->m_bAllowMoveDuringTaunt())
	{
		if (pLocal->m_bTauntForceMoveForward())
		{
			if (pCmd->buttons & IN_BACK)
				pCmd->viewangles.x = 91.f;
			else if (!(pCmd->buttons & IN_FORWARD))
				pCmd->viewangles.x = 90.f;
		}
		if (pCmd->buttons & IN_MOVELEFT)
			pCmd->sidemove = pCmd->viewangles.x != 90.f ? -50.f : -450.f;
		else if (pCmd->buttons & IN_MOVERIGHT)
			pCmd->sidemove = pCmd->viewangles.x != 90.f ? 50.f : 450.f;

		Vec3 vAngle = I::EngineClient->GetViewAngles();
		pCmd->viewangles.y = vAngle.y;

		G::SilentAngles = true;
	}
	else if (Vars::Misc::Automation::KartControl.Value && pLocal->InCond(TF_COND_HALLOWEEN_KART))
	{
		const bool bForward = pCmd->buttons & IN_FORWARD;
		const bool bBack = pCmd->buttons & IN_BACK;
		const bool bLeft = pCmd->buttons & IN_MOVELEFT;
		const bool bRight = pCmd->buttons & IN_MOVERIGHT;

		const bool flipVar = I::GlobalVars->tickcount % 2;
		if (bForward && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 0.f;
		}
		else if (bBack && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 91.f;
		}
		else if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		{
			if (flipVar || !F::Ticks.CanChoke())
			{	// you could just do this if you didn't care about viewangles
				const Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, 0.f);
				const float flLength = vecMove.Length();
				Vec3 angMoveReverse;
				Math::VectorAngles(vecMove * -1.f, angMoveReverse);
				pCmd->forwardmove = -flLength;
				pCmd->sidemove = 0.f;
				pCmd->viewangles.y = fmodf(pCmd->viewangles.y - angMoveReverse.y, 360.f);
				pCmd->viewangles.z = 270.f;
				G::PSilentAngles = true;
			}
		}
		else
			pCmd->viewangles.x = 90.f;

		G::SilentAngles = true;
	}
}

void CMisc::FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal->m_hGroundEntity() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	const float flSpeed = pLocal->m_vecVelocity().Length2D();
	const int flMaxSpeed = std::min(pLocal->m_flMaxspeed() * 0.9f, 520.f) - 10.f;
	const int iRun = !pCmd->forwardmove && !pCmd->sidemove ? 0 : flSpeed < flMaxSpeed ? 1 : 2;

	switch (iRun)
	{
	case 0:
	{
		if (!Vars::Misc::Movement::FastStop.Value || !flSpeed)
			return;

		Vec3 vDirection = pLocal->m_vecVelocity().ToAngle();
		vDirection.y = pCmd->viewangles.y - vDirection.y;
		Vec3 vNegatedDirection = vDirection.FromAngle() * -flSpeed;
		pCmd->forwardmove = vNegatedDirection.x;
		pCmd->sidemove = vNegatedDirection.y;

		break;
	}
	case 1:
	{
		if ((pLocal->IsDucking() ? !Vars::Misc::Movement::CrouchSpeed.Value : !Vars::Misc::Movement::FastAccel.Value)
			|| Vars::Misc::Game::AntiCheatCompatibility.Value
			|| G::Attacking == 1 || F::Ticks.m_bDoubletap || F::Ticks.m_bSpeedhack || F::Ticks.m_bRecharge || G::AntiAim || I::GlobalVars->tickcount % 2)
			return;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			return;

		Vec3 vMove = { pCmd->forwardmove, pCmd->sidemove, 0.f };
		float flLength = vMove.Length();
		Vec3 vAngMoveReverse; Math::VectorAngles(vMove * -1.f, vAngMoveReverse);
		pCmd->forwardmove = -flLength;
		pCmd->sidemove = 0.f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - vAngMoveReverse.y, 360.f);
		pCmd->viewangles.z = 270.f;
		G::PSilentAngles = true;

		break;
	}
	}
}

void CMisc::EdgeJump(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost)
{
	if (!Vars::Misc::Movement::EdgeJump.Value)
		return;

	static bool bStaticGround = false;
	if (!bPost)
		bStaticGround = pLocal->m_hGroundEntity();
	else if (bStaticGround && !pLocal->m_hGroundEntity())
		pCmd->buttons |= IN_JUMP;
}



void CMisc::Event(IGameEvent* pEvent, uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("client_disconnect"):
	case FNV1A::Hash32Const("client_beginconnect"):
	case FNV1A::Hash32Const("game_newmap"):
		m_iWishCmdrate = m_iWishUpdaterate = -1;
		F::Backtrack.m_flWishInterp = -1.f;
		[[fallthrough]];
	case FNV1A::Hash32Const("teamplay_round_start"):
		G::LineStorage.clear();
		G::BoxStorage.clear();
		G::PathStorage.clear();
		break;
	case FNV1A::Hash32Const("player_spawn"):
		m_bPeekPlaced = false;
	}
}

int CMisc::AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd, bool bSendPacket)
{
	if (!Vars::Misc::Automation::AntiBackstab.Value || !bSendPacket || G::Attacking == 1 || !pLocal || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return 0;

	std::vector<std::pair<Vec3, CBaseEntity*>> vTargets = {};
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
		if (!pWeapon
			|| pWeapon->GetWeaponID() != TF_WEAPON_KNIFE
			&& !(G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "crit_from_behind", pWeapon) > 0)
			&& !(pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER && SDK::AttribHookValue(0, "set_flamethrower_back_crit", pWeapon) == 1)
			|| F::PlayerUtils.IsIgnored(pPlayer->entindex()))
			continue;

		Vec3 vLocalPos = pLocal->GetCenter();
		Vec3 vTargetPos1 = pPlayer->GetCenter();
		Vec3 vTargetPos2 = vTargetPos1 + pPlayer->m_vecVelocity() * F::Backtrack.GetReal();
		float flDistance = std::max(std::max(SDK::MaxSpeed(pPlayer), SDK::MaxSpeed(pLocal)), pPlayer->m_vecVelocity().Length());
		if ((vLocalPos.DistTo(vTargetPos1) > flDistance || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos1))
			&& (vLocalPos.DistTo(vTargetPos2) > flDistance || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos2)))
			continue;

		vTargets.emplace_back(vTargetPos2, pEntity);
	}
	if (vTargets.empty())
		return 0;

	std::sort(vTargets.begin(), vTargets.end(), [&](const auto& a, const auto& b) -> bool
		{
			return pLocal->GetCenter().DistTo(a.first) < pLocal->GetCenter().DistTo(b.first);
		});

	auto& pTargetPos = vTargets.front();
	switch (Vars::Misc::Automation::AntiBackstab.Value)
	{
	case Vars::Misc::Automation::AntiBackstabEnum::Yaw:
	{
		Vec3 vAngleTo = Math::CalcAngle(pLocal->m_vecOrigin(), pTargetPos.first);
		vAngleTo.x = pCmd->viewangles.x;
		SDK::FixMovement(pCmd, vAngleTo);
		pCmd->viewangles = vAngleTo;
		
		return 1;
	}
	case Vars::Misc::Automation::AntiBackstabEnum::Pitch:
	case Vars::Misc::Automation::AntiBackstabEnum::Fake:
	{
		bool bCheater = F::PlayerUtils.HasTag(pTargetPos.second->entindex(), F::PlayerUtils.TagToIndex(CHEATER_TAG));
		// if the closest spy is a cheater, assume auto stab is being used, otherwise don't do anything if target is in front
		if (!bCheater)
		{
			auto TargetIsBehind = [&]()
				{
					Vec3 vToTarget = pLocal->m_vecOrigin() - pTargetPos.first;
					vToTarget.z = 0.f;
					const float flDist = vToTarget.Length();
					if (!flDist)
						return true;

					Vec3 vTargetAngles = pCmd->viewangles;

					vToTarget.Normalize();
					float flTolerance = 0.0625f;
					float flExtra = 2.f * flTolerance / flDist; // account for origin tolerance

					float flPosVsTargetViewMinDot = 0.f - 0.0031f - flExtra;

					Vec3 vTargetForward; Math::AngleVectors(vTargetAngles, &vTargetForward);
					vTargetForward.z = 0.f;
					vTargetForward.Normalize();

					return vToTarget.Dot(vTargetForward) > flPosVsTargetViewMinDot;
				};

			if (!TargetIsBehind())
				return 0;
		}

		if (!bCheater || Vars::Misc::Automation::AntiBackstab.Value == Vars::Misc::Automation::AntiBackstabEnum::Pitch)
		{
			pCmd->forwardmove *= -1;
			pCmd->viewangles.x = 269.f;
		}
		else
		{
			pCmd->viewangles.x = 271.f;
		}
		// may slip up some auto backstabs depending on mode, though we are still able to be stabbed

		return 2;
	}
	}

	return 0;
}

void CMisc::UnlockAchievements()
{
	const auto pAchievementMgr = reinterpret_cast<IAchievementMgr*(*)(void)>(U::Memory.GetVFunc(I::EngineClient, 114))();
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			pAchievementMgr->AwardAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::LockAchievements()
{
	const auto pAchievementMgr = reinterpret_cast<IAchievementMgr*(*)(void)>(U::Memory.GetVFunc(I::EngineClient, 114))();
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			I::SteamUserStats->ClearAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetName());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

bool CMisc::SteamRPC()
{
	/*
	if (!Vars::Misc::Steam::EnableRPC.Value)
	{
		if (!bSteamCleared) // stupid way to return back to normal rpc
		{
			I::SteamFriends->SetRichPresence("steam_display", ""); // this will only make it say "Team Fortress 2" until the player leaves/joins some server. its bad but its better than making 1000 checks to recreate the original
			bSteamCleared = true;
		}
		return false;
	}

	bSteamCleared = false;
	*/


	if (!Vars::Misc::Steam::EnableRPC.Value)
		return false;

	I::SteamFriends->SetRichPresence("steam_display", "#TF_RichPresence_Display");
	if (!I::EngineClient->IsInGame() && !Vars::Misc::Steam::OverrideMenu.Value)
		I::SteamFriends->SetRichPresence("state", "MainMenu");
	else
	{
		I::SteamFriends->SetRichPresence("state", "PlayingMatchGroup");

		switch (Vars::Misc::Steam::MatchGroup.Value)
		{
		case Vars::Misc::Steam::MatchGroupEnum::SpecialEvent: I::SteamFriends->SetRichPresence("matchgrouploc", "SpecialEvent"); break;
		case Vars::Misc::Steam::MatchGroupEnum::MvMMannUp: I::SteamFriends->SetRichPresence("matchgrouploc", "MannUp"); break;
		case Vars::Misc::Steam::MatchGroupEnum::Competitive: I::SteamFriends->SetRichPresence("matchgrouploc", "Competitive6v6"); break;
		case Vars::Misc::Steam::MatchGroupEnum::Casual: I::SteamFriends->SetRichPresence("matchgrouploc", "Casual"); break;
		case Vars::Misc::Steam::MatchGroupEnum::MvMBootCamp: I::SteamFriends->SetRichPresence("matchgrouploc", "BootCamp"); break;
		}
	}
	I::SteamFriends->SetRichPresence("currentmap", Vars::Misc::Steam::MapText.Value.c_str());
	I::SteamFriends->SetRichPresence("steam_player_group_size", std::to_string(Vars::Misc::Steam::GroupSize.Value).c_str());

	return true;
}