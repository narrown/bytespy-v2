#include "AimbotGlobal.h"

#include "../../Players/PlayerUtils.h"

void CAimbotGlobal::SortTargets(std::vector<Target_t>* targets, int method)
{
	auto comparator = [method](const Target_t& a, const Target_t& b)
		{
			if (method == Vars::Aimbot::General::TargetSelectionEnum::FOV)
				return a.m_flFOVTo < b.m_flFOVTo;

			if (method == Vars::Aimbot::General::TargetSelectionEnum::Distance)
				return a.m_flDistTo < b.m_flDistTo;

			return false;
		};

	std::sort(targets->begin(), targets->end(), comparator);
}

void CAimbotGlobal::SortPriority(std::vector<Target_t>* targets)
{
	auto comparePriority = [](const Target_t& lhs, const Target_t& rhs)
		{
			return lhs.m_nPriority > rhs.m_nPriority;
		};

	std::sort(targets->begin(), targets->end(), comparePriority);
}

bool CAimbotGlobal::PlayerBoneInFOV(CTFPlayer* target, Vec3 localPos, Vec3 localAngles, float& bestFOV, Vec3& bestPos, Vec3& bestAngle, int hitboxMask)
{
	float minFOV = 180.f;
	const int hitboxCount = target->GetNumOfHitboxes();

	for (int i = 0; i < hitboxCount; ++i)
	{
		if (!IsHitboxValid(H::Entities.GetModel(target->entindex()), i, hitboxMask))
			continue;

		Vec3 pos = target->GetHitboxCenter(i);
		Vec3 angle = Math::CalcAngle(localPos, pos);
		float fov = Math::CalcFov(localAngles, angle);

		if (fov >= minFOV)
			continue;

		minFOV = fov;
		bestFOV = fov;
		bestPos = pos;
		bestAngle = angle;
	}

	return minFOV < Vars::Aimbot::General::AimFOV.Value;
}

bool CAimbotGlobal::IsHitboxValid(uint32_t modelHash, int hitbox, int hitboxFlags)
{
	const bool isSaxton = modelHash == FNV1A::Hash32Const("models/vsh/player/saxton_hale.mdl");
	uint32_t staticmask = Vars::Aimbot::Hitscan::StaticHitboxes.Value;

	if (hitbox == -1)
		return true;

	if (staticmask)
	{
		uint32_t boneBit = 0;
		switch (hitbox)
		{
		case HITBOX_HEAD:
			boneBit = Vars::Aimbot::Hitscan::HitboxesEnum::Head;
			break;
		case HITBOX_PELVIS:
			boneBit = Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis;
			break;
		case HITBOX_BODY:
		case HITBOX_THORAX:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			boneBit = Vars::Aimbot::Hitscan::HitboxesEnum::Body;
			break;
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
			boneBit = Vars::Aimbot::Hitscan::HitboxesEnum::Arms;
			break;
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
			boneBit = Vars::Aimbot::Hitscan::HitboxesEnum::Legs;
			break;
		default:
			boneBit = 0;
		}
		return boneBit && ((staticmask & boneBit) != 0) && ((hitboxFlags & boneBit) != 0);
	}

	if (isSaxton)
	{
		if (hitbox == HITBOX_SAXTON_HEAD)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Head) != 0;
		if (hitbox == HITBOX_SAXTON_PELVIS)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis) != 0;
		if (hitbox == HITBOX_SAXTON_BODY ||
			hitbox == HITBOX_SAXTON_THORAX ||
			hitbox == HITBOX_SAXTON_CHEST ||
			hitbox == HITBOX_SAXTON_UPPER_CHEST ||
			hitbox == HITBOX_SAXTON_NECK)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Body) != 0;
		if (hitbox == HITBOX_SAXTON_LEFT_UPPER_ARM ||
			hitbox == HITBOX_SAXTON_LEFT_FOREARM ||
			hitbox == HITBOX_SAXTON_LEFT_HAND ||
			hitbox == HITBOX_SAXTON_RIGHT_UPPER_ARM ||
			hitbox == HITBOX_SAXTON_RIGHT_FOREARM ||
			hitbox == HITBOX_SAXTON_RIGHT_HAND)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Arms) != 0;
		if (hitbox == HITBOX_SAXTON_LEFT_THIGH ||
			hitbox == HITBOX_SAXTON_LEFT_CALF ||
			hitbox == HITBOX_SAXTON_LEFT_FOOT ||
			hitbox == HITBOX_SAXTON_RIGHT_THIGH ||
			hitbox == HITBOX_SAXTON_RIGHT_CALF ||
			hitbox == HITBOX_SAXTON_RIGHT_FOOT)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Legs) != 0;
	}
	else
	{
		if (hitbox == HITBOX_HEAD)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Head) != 0;
		if (hitbox == HITBOX_PELVIS)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis) != 0;
		if (hitbox == HITBOX_BODY ||
			hitbox == HITBOX_THORAX ||
			hitbox == HITBOX_CHEST ||
			hitbox == HITBOX_UPPER_CHEST)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Body) != 0;
		if (hitbox == HITBOX_LEFT_UPPER_ARM ||
			hitbox == HITBOX_LEFT_FOREARM ||
			hitbox == HITBOX_LEFT_HAND ||
			hitbox == HITBOX_RIGHT_UPPER_ARM ||
			hitbox == HITBOX_RIGHT_FOREARM ||
			hitbox == HITBOX_RIGHT_HAND)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Arms) != 0;
		if (hitbox == HITBOX_LEFT_THIGH ||
			hitbox == HITBOX_LEFT_CALF ||
			hitbox == HITBOX_LEFT_FOOT ||
			hitbox == HITBOX_RIGHT_THIGH ||
			hitbox == HITBOX_RIGHT_CALF ||
			hitbox == HITBOX_RIGHT_FOOT)
			return (hitboxFlags & Vars::Aimbot::Hitscan::HitboxesEnum::Legs) != 0;
	}

	return false;
}

bool CAimbotGlobal::ShouldIgnore(CBaseEntity* ent, CTFPlayer* local, CTFWeaponBase* weapon)
{
	if (ent->IsDormant())
		return true;

	if (const auto rules = I::TFGameRules())
		if (rules->m_bTruceActive() && local->m_iTeamNum() != ent->m_iTeamNum())
			return true;

	const auto classID = ent->GetClassID();

	if (classID == ETFClassID::CTFPlayer)
	{
		auto player = ent->As<CTFPlayer>();
		if (!player || player == local || !player->IsAlive() || player->IsAGhost())
			return true;

		if (local->m_iTeamNum() == player->m_iTeamNum())
			return false;

		const int index = player->entindex();
		const int ignoreFlags = Vars::Aimbot::General::Ignore.Value;

		if (F::PlayerUtils.IsIgnored(index))
			return true;

		if ((ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Friends && H::Entities.IsFriend(index)) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Party && H::Entities.InParty(index)) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Invulnerable && player->IsInvulnerable() && SDK::AttribHookValue(0, "crit_forces_victim_to_laugh", weapon) <= 0) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Cloaked && player->IsInvisible() && player->GetInvisPercentage() >= Vars::Aimbot::General::IgnoreCloakPercentage.Value) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::DeadRinger && player->m_bFeignDeathReady()) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Taunting && player->IsTaunting()) ||
			(ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Disguised && player->InCond(TF_COND_DISGUISED)))
			return true;

		if (ignoreFlags & Vars::Aimbot::General::IgnoreEnum::Vaccinator)
		{
			switch (G::PrimaryWeaponType)
			{
			case EWeaponType::HITSCAN:
				if (player->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) && SDK::AttribHookValue(0, "mod_pierce_resists_absorbs", weapon) != 0)
					return true;
				break;

			case EWeaponType::PROJECTILE:
				switch (weapon->GetWeaponID())
				{
				case TF_WEAPON_FLAMETHROWER:
				case TF_WEAPON_FLAREGUN:
					if (player->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST))
						return true;
					break;

				case TF_WEAPON_COMPOUND_BOW:
					if (player->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST))
						return true;
					break;

				default:
					if (player->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST))
						return true;
					break;
				}
				break;
			}
		}

		return false;
	}

	if (classID == ETFClassID::CObjectSentrygun || classID == ETFClassID::CObjectDispenser || classID == ETFClassID::CObjectTeleporter)
	{
		auto obj = ent->As<CBaseObject>();

		if ((obj->IsSentrygun() && !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry)) ||
			(obj->IsDispenser() && !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser)) ||
			(obj->IsTeleporter() && !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter)))
			return true;

		if (local->m_iTeamNum() == ent->m_iTeamNum())
			return false;

		const auto builder = obj->m_hBuilder().Get();
		if (builder)
		{
			const int ownerIdx = builder->entindex();
			if (F::PlayerUtils.IsIgnored(ownerIdx))
				return true;

			if ((Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Friends && H::Entities.IsFriend(ownerIdx)) ||
				(Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Party && H::Entities.InParty(ownerIdx)))
				return true;
		}

		return false;
	}

	if (classID == ETFClassID::CTFGrenadePipebombProjectile)
	{
		auto proj = ent->As<CTFGrenadePipebombProjectile>();

		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies) ||
			local->m_iTeamNum() == ent->m_iTeamNum())
			return true;

		auto owner = proj->m_hThrower().Get();
		if ((owner && F::PlayerUtils.IsIgnored(owner->entindex())) || proj->m_iType() != TF_GL_MODE_REMOTE_DETONATE || !proj->m_bTouched())
			return true;

		return false;
	}

	if (classID == ETFClassID::CEyeballBoss || classID == ETFClassID::CHeadlessHatman || classID == ETFClassID::CMerasmus || classID == ETFClassID::CTFBaseBoss)
		return !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs) || ent->m_iTeamNum() != TF_TEAM_HALLOWEEN;

	if (classID == ETFClassID::CTFTankBoss || classID == ETFClassID::CZombie)
		return !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs) || local->m_iTeamNum() == ent->m_iTeamNum();

	if (classID == ETFClassID::CTFPumpkinBomb || classID == ETFClassID::CTFGenericBomb)
		return !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs);

	return true;
}

int CAimbotGlobal::GetPriority(int targetIdx)
{
	return F::PlayerUtils.GetPriority(targetIdx);
}

bool CAimbotGlobal::ValidBomb(CTFPlayer* local, CTFWeaponBase* weapon, CBaseEntity* bomb)
{
	if (G::PrimaryWeaponType == EWeaponType::PROJECTILE)
		return false;

	const auto origin = bomb->m_vecOrigin();
	CEntitySphereQuery query(origin, 300.f);
	CBaseEntity* entity = nullptr;

	while ((entity = query.GetCurrentEntity()) != nullptr)
	{
		query.NextEntity();

		if (!entity || entity == local)
			continue;

		const bool isPly = entity->IsPlayer();
		if (isPly)
		{
			auto tfPlayer = entity->As<CTFPlayer>();
			if (!tfPlayer->IsAlive() || tfPlayer->IsAGhost() || entity->m_iTeamNum() == local->m_iTeamNum())
				continue;
		}
		else if (entity->m_iTeamNum() == local->m_iTeamNum())
		{
			continue;
		}

		Vec3 closest{};
		reinterpret_cast<CCollisionProperty*>(entity->GetCollideable())->CalcNearestPoint(origin, &closest);
		if (origin.DistTo(closest) > 300.f)
			continue;

		const auto flags = Vars::Aimbot::General::Target.Value;

		const bool targetPlayer = isPly && (flags & Vars::Aimbot::General::TargetEnum::Players);
		const bool targetSentry = entity->IsSentrygun() && (flags & Vars::Aimbot::General::TargetEnum::Sentry);
		const bool targetDisp = entity->IsDispenser() && (flags & Vars::Aimbot::General::TargetEnum::Dispenser);
		const bool targetTp = entity->IsTeleporter() && (flags & Vars::Aimbot::General::TargetEnum::Teleporter);
		const bool targetNpc = entity->IsNPC() && (flags & Vars::Aimbot::General::TargetEnum::NPCs);

		if (!(targetPlayer || targetSentry || targetDisp || targetTp || targetNpc))
			continue;

		if (targetPlayer && ShouldIgnore(entity->As<CTFPlayer>(), local, weapon))
			continue;

		const Vec3 endPos = targetPlayer ? entity->m_vecOrigin() + entity->As<CTFPlayer>()->GetViewOffset() : entity->GetCenter();
		if (!SDK::VisPosProjectile(bomb, entity, origin, endPos, MASK_SHOT))
			continue;

		return true;
	}

	return false;
}
