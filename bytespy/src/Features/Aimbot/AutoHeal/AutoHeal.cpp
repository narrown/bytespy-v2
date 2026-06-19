#include "AutoHeal.h"

#include "../../Players/PlayerUtils.h"
#include "../../Aimbot/Aimbot.h"
#include "../../Visuals/PlayerConditions/PlayerConditions.h"
#include "../../Backtrack/Backtrack.h"
#include "../../PacketManip/AntiAim/AntiAim.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../CameraWindow/CameraWindow.h"
#include "../../NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../../Spectate/Spectate.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Output/Output.h"
#include <string>

constexpr float HEALTH_LIMIT = 0.85f;
constexpr float ROCKET_BLAST_RADIUS = 146.0f; // Approximately the blast radius of a stock rocket
constexpr float ARROW_HITBOX_RADIUS = 30.0f;  // A reasonable distance for an arrow direct hit

bool CAutoHeal::ActivateOnVoice(CTFPlayer* pLocal, CWeaponMedigun* pWeapon, CUserCmd* pCmd)
{
    if (!Vars::Aimbot::Healing::ActivateOnVoice.Value)
        return false;

    if (!pWeapon || pWeapon->m_hHealingTarget().Get())
        return false;

    if (!pWeapon || !pWeapon->m_hHealingTarget().Get())
        return false;

    auto pTarget = pWeapon->m_hHealingTarget().Get()->As<CTFPlayer>();
    if (!pTarget || (Vars::Aimbot::Healing::FriendsOnly.Value && !H::Entities.IsFriend(pTarget->entindex()) && !H::Entities.InParty(pTarget->entindex())))
        return false;

    bool bReturn = m_mMedicCallers.contains(pTarget->entindex());
    if (bReturn)
        pCmd->buttons |= IN_ATTACK2;

    return bReturn;
}

medigun_resist_types_t WeaponIDToResType(int weaponID)
{
    switch (weaponID)
    {
    case TF_WEAPON_SHOTGUN_PRIMARY:
    case TF_WEAPON_SHOTGUN_SOLDIER:
    case TF_WEAPON_SHOTGUN_HWG:
    case TF_WEAPON_SHOTGUN_PYRO:
    case TF_WEAPON_SCATTERGUN:
    case TF_WEAPON_SNIPERRIFLE:
    case TF_WEAPON_MINIGUN:
    case TF_WEAPON_SMG:
    case TF_WEAPON_PISTOL:
    case TF_WEAPON_PISTOL_SCOUT:
    case TF_WEAPON_REVOLVER:
    case TF_WEAPON_SENTRY_BULLET:
    case TF_WEAPON_COMPOUND_BOW:
    case TF_WEAPON_SENTRY_REVENGE:
    case TF_WEAPON_HANDGUN_SCOUT_PRIMARY:
    case TF_WEAPON_CROSSBOW:
    case TF_WEAPON_HANDGUN_SCOUT_SECONDARY:
    case TF_WEAPON_SODA_POPPER:
    case TF_WEAPON_SNIPERRIFLE_DECAP:
    case TF_WEAPON_PEP_BRAWLER_BLASTER:
    case TF_WEAPON_CHARGED_SMG:
    case TF_WEAPON_SNIPERRIFLE_CLASSIC:
    case TF_WEAPON_RAYGUN:
    case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
    case TF_WEAPON_DRG_POMSON:
    case TF_WEAPON_SYRINGEGUN_MEDIC:
    case TF_WEAPON_LASER_POINTER:
    {
        return MEDIGUN_BULLET_RESIST;
    }

    case TF_WEAPON_ROCKETLAUNCHER:
    case TF_WEAPON_GRENADELAUNCHER:
    case TF_WEAPON_PIPEBOMBLAUNCHER:
    case TF_WEAPON_FLAMETHROWER_ROCKET:
    case TF_WEAPON_GRENADE_DEMOMAN:
    case TF_WEAPON_SENTRY_ROCKET:
    case TF_WEAPON_PUMPKIN_BOMB:
    case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
    case TF_WEAPON_CANNON:
    case TF_WEAPON_PARTICLE_CANNON:
    {
        return MEDIGUN_BLAST_RESIST;
    }

    case TF_WEAPON_FLAMETHROWER:
    case TF_WEAPON_FLAREGUN:
    case TF_WEAPON_FLAREGUN_REVENGE:
    case TF_WEAPON_FLAME_BALL:
    {
        return MEDIGUN_FIRE_RESIST;
    }

    default:
    {
        return MEDIGUN_NUM_RESISTS;
    }
    }
}

bool TraceEntityAutoDet(CBaseEntity* pEntity, const Vec3& vFrom, const Vec3& vTo)
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal) return false;
    CGameTrace trace;
    CTraceFilterProjectile filter;
    filter.pSkip = pLocal;
    Ray_t ray;
    ray.Init(vFrom, vTo);
    I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);
    return (trace.m_pEnt == pEntity) || (trace.fraction > 0.99f);
}

bool IsPlayerInDanger(CTFPlayer * player, medigun_resist_types_t & dangerType, std::string * dangerReason = nullptr)
{
    if (!player || player->IsInvulnerable())
        return false;

    const float percentHealth = Math::RemapValClamped(
        static_cast<float>(player->m_iHealth()), 0.0f,
        static_cast<float>(player->GetMaxHealth()), 0.0f, 1.0f
    );

    // --- Player vischeck ---
    for (const auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
    {
        if (!pEntity)
            continue;

        const auto enemy = pEntity->As<CTFPlayer>();
        if (!enemy || enemy->deadflag())
            continue;

        // Check if the enemy is visible to the player or vice-versa before considering them a threat.
        if (!SDK::VisPos(enemy, player, enemy->GetShootPos(), player->GetShootPos()) && !SDK::VisPos(player, enemy, player->GetShootPos(), enemy->GetShootPos()))
            continue;

        const int iClass = enemy->m_iClass();
        bool bCheater = F::PlayerUtils.HasTag(enemy->entindex(), F::PlayerUtils.TagToIndex(CHEATER_TAG));
        bool bDangerousCheater = false;

        const float flDist = player->GetAbsOrigin().DistTo(enemy->GetAbsOrigin());

        switch (iClass)
        {
        case TF_CLASS_SCOUT:
        {
            if (bCheater && flDist <= 350.0f)
            {
                if (dangerReason) *dangerReason = "cheater in lethal dt range";
                dangerType = MEDIGUN_BULLET_RESIST;
                return true;
            }
            break;
        }

        case TF_CLASS_HEAVY:
        {
            if (flDist <= 400.0f)
            {
                if (bCheater && enemy->InCond(TF_COND_AIMING))
                {
                    if (dangerReason) *dangerReason = "cheater in lethal dt range";
                    dangerType = MEDIGUN_BULLET_RESIST;
                    return true;
                }
                else if (enemy->InCond(TF_COND_AIMING))
                {
                    if (dangerReason) *dangerReason = "spun-up heavy";
                    dangerType = MEDIGUN_BULLET_RESIST;
                    return true;
                }
            }
            bDangerousCheater = bCheater && enemy->InCond(TF_COND_AIMING);
            break;
        }

        case TF_CLASS_SNIPER:
        {
            if (!bCheater)
                break;

            const auto weapon = enemy->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
            if (!weapon || weapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
                break;

            bool zoomed = enemy->InCond(TF_COND_ZOOMED);
            if (weapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
                zoomed = weapon->As<CTFSniperRifleClassic>()->m_bCharging();

            bDangerousCheater = bCheater || zoomed || enemy->IsCritBoosted() || enemy->IsMiniCritBoosted();
            if (!bDangerousCheater)
                break;

            Vec3 forward{};
            Math::AngleVectors(enemy->GetEyeAngles(), &forward);
            Vec3 rayStart = enemy->GetShootPos();

            if (!Math::RayToOBB(rayStart, forward,
                player->GetCollideable()->OBBMins(),
                player->GetCollideable()->OBBMaxs(),
                player->RenderableToWorldTransform()))
            {
                break;
            }

            if (dangerReason) *dangerReason = "sniper aim";
            dangerType = MEDIGUN_BULLET_RESIST;
            return true;
        }

        default:
            break;
        }

        if (bDangerousCheater)
        {
            if (dangerReason) *dangerReason = "cheater too close";
            dangerType = MEDIGUN_BULLET_RESIST;
            return true;
        }
    }

    size_t numClosePipebombs{};

    for (const auto pEntity : H::Entities.GetGroup(EGroupType::PROJECTILES_ENEMIES))
    {
        if (!pEntity)
            continue;

        // Use the new CProjectileSimulation class for prediction
        ProjectileInfo projInfo;

        // Get the projectile's owner and owner's weapon
        const auto pOwner = pEntity->m_hOwnerEntity().Get()->As<CTFPlayer>();
        if (!pOwner) continue;

        // The GetInfo function takes the weapon from the owner, so we don't need the projectile's weapon
        const auto pWeapon = pOwner->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
        if (!pWeapon) continue;

        // Set the projectile's position and angles
        projInfo.m_vPos = pEntity->m_vecOrigin();
        projInfo.m_vAng = pEntity->GetAbsAngles();
        projInfo.m_pOwner = pOwner;

        if (!F::ProjSim.GetInfo(pOwner, pWeapon, pEntity->GetAbsAngles(), projInfo, ProjSimEnum::InitCheck)) {
            continue;
        }

        // Now, simulate the projectile's path.
        if (!F::ProjSim.Initialize(projInfo, true, true)) {
            continue; // Could not initialize
        }

        // We'll simulate for up to 2 seconds, which is a good safety margin
        for (float flSimTime = 0.0f; flSimTime < 2.0f; flSimTime += I::GlobalVars->interval_per_tick) {

            // Run a tick of the simulation
            F::ProjSim.RunTick(projInfo, true);

            // Get the simulated projectile's position
            if (projInfo.m_vPath.empty()) {
                break; // Simulation ended
            }

            const Vec3& simPoint = projInfo.m_vPath.back();

            // Predict the player's future position
            // Use the GetAbsVelocity() function from the provided CBaseEntity header.
            const Vec3 predictedPlayerPos = player->GetAbsOrigin() + player->GetAbsVelocity() * flSimTime;
            const Vec3 predictedPlayerCenter = player->GetCenter() + player->GetAbsVelocity() * flSimTime;

            float dangerDistance = 0.0f;

            // Determine the danger distance based on projectile type
            switch (pEntity->GetClassID())
            {
            case ETFClassID::CTFProjectile_Rocket:
            case ETFClassID::CTFProjectile_SentryRocket:
            case ETFClassID::CTFGrenadePipebombProjectile:
            case ETFClassID::CTFProjectile_EnergyBall:
                dangerDistance = ROCKET_BLAST_RADIUS;
                break;
            case ETFClassID::CTFProjectile_Arrow:
            case ETFClassID::CTFProjectile_HealingBolt:
            case ETFClassID::CTFProjectile_Flare:
            case ETFClassID::CTFProjectile_EnergyRing:
            case ETFClassID::CTFProjectile_BallOfFire:
                dangerDistance = ARROW_HITBOX_RADIUS;
                break;
            default:
                dangerDistance = 50.0f; // Default for unknown projectiles
                break;
            }

            if (simPoint.DistTo(predictedPlayerCenter) < dangerDistance) {
                // If the projectile is predicted to be close, check for visibility
                const auto visibleFromCenter{ TraceEntityAutoDet(pEntity, predictedPlayerCenter, simPoint) };
                const auto visibleFromHead{ TraceEntityAutoDet(pEntity, predictedPlayerPos + Vec3{ 0.0f, 0.0f, player->m_vecMaxs().z }, simPoint) };

                if (visibleFromCenter || visibleFromHead) {
                    // We have a predicted projectile that will be in range. Now check its type.
                    switch (pEntity->GetClassID())
                    {
                    case ETFClassID::CTFProjectile_Arrow:
                    {
                        auto arrow = pEntity->As<CTFProjectile_Arrow>();
                        if (arrow && arrow->m_bCritical()) {
                            dangerType = MEDIGUN_BULLET_RESIST;
                            if (dangerReason) *dangerReason = "incoming crit arrow";
                            return true;
                        }
                        if (percentHealth < HEALTH_LIMIT) {
                            dangerType = MEDIGUN_BULLET_RESIST;
                            if (dangerReason) *dangerReason = "incoming arrow (low health)";
                            return true;
                        }
                        continue;
                    }

                    case ETFClassID::CTFProjectile_HealingBolt:
                    {
                        const auto arrow{ pEntity->As<CTFProjectile_Arrow>() };
                        if (!arrow || (!arrow->m_bCritical() && percentHealth >= HEALTH_LIMIT))
                            continue;

                        dangerType = MEDIGUN_BULLET_RESIST;
                        if (dangerReason) *dangerReason = "incoming healing bolt";
                        return true;
                    }

                    case ETFClassID::CTFProjectile_Rocket:
                    case ETFClassID::CTFProjectile_SentryRocket:
                    case ETFClassID::CTFProjectile_EnergyBall:
                    {
                        const auto rocket{ pEntity->As<CTFProjectile_Rocket>() };
                        if (rocket && !rocket->m_bCritical() && percentHealth >= 1.0f)
                            continue;

                        dangerType = MEDIGUN_BLAST_RESIST;
                        if (dangerReason) *dangerReason = "incoming rocket";
                        return true;
                    }

                    case ETFClassID::CTFGrenadePipebombProjectile:
                    {
                        const auto bomb{ pEntity->As<CTFGrenadePipebombProjectile>() };
                        if (!bomb || bomb->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE)
                            continue;

                        if (bomb->m_iType() == TF_GL_MODE_REMOTE_DETONATE) {
                            if (simPoint.DistTo(predictedPlayerCenter) < 50.0f) {
                                numClosePipebombs++;
                            }
                            continue;
                        }

                        if (!bomb->m_bCritical() && percentHealth >= 1.0f && simPoint.DistTo(predictedPlayerCenter) >= 100.0f)
                        {
                            continue;
                        }

                        dangerType = MEDIGUN_BLAST_RESIST;
                        if (dangerReason) *dangerReason = "incoming grenade";
                        return true;
                    }

                    case ETFClassID::CTFProjectile_Flare:
                    {
                        const auto flare{ pEntity->As<CTFProjectile_Flare>() };
                        if (!flare || (!flare->m_bCritical() && !player->InCond(TF_COND_BURNING) && !player->InCond(TF_COND_BURNING_PYRO)))
                            continue;

                        dangerType = MEDIGUN_FIRE_RESIST;
                        if (dangerReason) *dangerReason = "incoming flare";
                        return true;
                    }

                    case ETFClassID::CTFProjectile_BallOfFire:
                    {
                        if (!player->InCond(TF_COND_BURNING) && !player->InCond(TF_COND_BURNING_PYRO))
                            continue;

                        dangerType = MEDIGUN_FIRE_RESIST;
                        if (dangerReason) *dangerReason = "incoming fireball";
                        return true;
                    }

                    default: {}
                    }
                }
            }
        }
    }

    if (numClosePipebombs > 0)
    {
        dangerType = MEDIGUN_BLAST_RESIST;
        if (numClosePipebombs > 1 || (numClosePipebombs == 1 && percentHealth < 0.5f)) {
            return true;
        }
    }

    // --- Sentry vischeck ---
    for (const auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ENEMIES))
    {
        if (!pEntity || pEntity->GetClassID() != ETFClassID::CObjectSentrygun)
            continue;

        CObjectSentrygun* sentrygun{ pEntity->As<CObjectSentrygun>() };
        if (!sentrygun || !sentrygun->m_bPlayerControlled())
            continue;

        // Check if the sentry can see the player
        if (!SDK::VisPos(sentrygun, player, sentrygun->GetAbsOrigin(), player->GetShootPos()))
            continue;

        if (sentrygun->m_hAutoAimTarget() != player && sentrygun->m_hEnemy() != player)
            continue;

        CGameTrace trace;
        Ray_t ray;
        CTraceFilterWorldAndPropsOnly filter;
        Vec3 eyePos = sentrygun->GetAbsOrigin() + Vec3(0, 0, 50);
        ray.Init(eyePos, player->GetCenter());
        I::EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

        if (trace.m_pEnt != player && trace.fraction < 0.99f) {
            continue;
        }


        dangerType = MEDIGUN_BULLET_RESIST;

        if (percentHealth < HEALTH_LIMIT)
            return true;
    }

    return false;
}

bool PlayerHasResUber(medigun_resist_types_t res, CTFPlayer * player)
{
    if (!player) return false;
    switch (res)
    {
    case MEDIGUN_BULLET_RESIST:
        return player->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST);

    case MEDIGUN_BLAST_RESIST:
        return player->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST);

    case MEDIGUN_FIRE_RESIST:
        return player->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST);

    default:
        return false;
    }
}

void CAutoHeal::Reset()
{
    m_GoalResType = MEDIGUN_NUM_RESISTS;
    m_IsChangingRes = false;
    m_ShouldPop = false;
    m_SimResType = MEDIGUN_NUM_RESISTS;
}

void CAutoHeal::Run(CTFPlayer * pLocal, CTFWeaponBase * pWeapon, CUserCmd * pCmd)
{
    bool bActivatedByVoice = ActivateOnVoice(pLocal, pWeapon ? pWeapon->As<CWeaponMedigun>() : nullptr, pCmd);
    m_mMedicCallers.clear();
    if (bActivatedByVoice) {
        return;
    }

    if (!Vars::Misc::Automation::AutoVaccinator_Active.Value) {
        if (m_GoalResType != MEDIGUN_NUM_RESISTS || m_IsChangingRes || m_ShouldPop) {
            Reset();
        }
        return;
    }

    if (!pLocal || pLocal->deadflag() || !pWeapon) {
        Reset();
        return;
    }

    if (pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN) {
        Reset();
        return;
    }

    const auto medigun = pWeapon->As<CWeaponMedigun>();
    if (!medigun || medigun->GetChargeType() < 3) {
        Reset();
        return;
    }

    const auto currentActualResType = medigun->GetResistType();
    if (m_SimResType == MEDIGUN_NUM_RESISTS || (!m_IsChangingRes && m_SimResType != currentActualResType)) {
        m_SimResType = currentActualResType;
    }

    if (m_IsChangingRes) {
        if (m_SwitchDelayTicks > 0) {
            --m_SwitchDelayTicks;
        }
        else if (m_ResSwitchesRemaining > 0) {
            pCmd->buttons |= IN_RELOAD;
            --m_ResSwitchesRemaining;
            m_SimResType = (m_SimResType + 1) % 3;
            m_SwitchDelayTicks = 2;
        }
        else {
            m_IsChangingRes = false;
        }

        if (pCmd->buttons & IN_RELOAD) {
            return;
        }
    }

    if (!m_IsChangingRes && m_ShouldPop) {
        const char* resStrForPop =
            m_GoalResType == MEDIGUN_BULLET_RESIST ? "bullet" :
            m_GoalResType == MEDIGUN_BLAST_RESIST ? "blast" :
            m_GoalResType == MEDIGUN_FIRE_RESIST ? "fire" : "unknown";

        F::Output.ReportVaccinator(medigun->m_hHealingTarget().Get() ? medigun->m_hHealingTarget().Get()->entindex() : pLocal->entindex(), "popped", resStrForPop, "danger nearby");

        if (currentActualResType == m_GoalResType) {
            pCmd->buttons |= IN_ATTACK2;
            m_ShouldPop = false;
            m_GoalResType = MEDIGUN_NUM_RESISTS;
            return;
        }
        else {
            m_IsChangingRes = true;
            const int targetRes = static_cast<int>(m_GoalResType);
            m_ResSwitchesRemaining = (targetRes - currentActualResType + MEDIGUN_NUM_RESISTS) % MEDIGUN_NUM_RESISTS;
            m_SwitchDelayTicks = 0;
            return;
        }
    }

    if (!m_IsChangingRes && !m_ShouldPop && !(pCmd->buttons & (IN_ATTACK2 | IN_RELOAD))) {
        const auto healTarget = medigun->m_hHealingTarget().Get() ?
            medigun->m_hHealingTarget().Get()->As<CTFPlayer>() : nullptr;

        medigun_resist_types_t detectedDanger = MEDIGUN_NUM_RESISTS;
        CTFPlayer* dangerSubject = nullptr;
        bool shouldPopNow = false;
        std::string dangerReasonStr = "deadly threat";

        if (healTarget && IsPlayerInDanger(healTarget, detectedDanger, &dangerReasonStr) &&
            !PlayerHasResUber(detectedDanger, healTarget)) {
            dangerSubject = healTarget;
            bool isAllowed = !Vars::Aimbot::Healing::FriendsOnly.Value ||
                H::Entities.IsFriend(healTarget->entindex());
            shouldPopNow = isAllowed && medigun->m_flChargeLevel() >= 0.25f;
        }
        else if (IsPlayerInDanger(pLocal, detectedDanger, &dangerReasonStr) &&
            !PlayerHasResUber(detectedDanger, pLocal)) {
            dangerSubject = pLocal;
            shouldPopNow = medigun->m_flChargeLevel() >= 0.25f;
        }

        if (dangerSubject && detectedDanger != MEDIGUN_NUM_RESISTS) {
            m_GoalResType = detectedDanger;
            m_ShouldPop = shouldPopNow;

            const char* resStr = detectedDanger == MEDIGUN_BULLET_RESIST ? "bullet" :
                detectedDanger == MEDIGUN_BLAST_RESIST ? "blast" :
                detectedDanger == MEDIGUN_FIRE_RESIST ? "fire" : "unknown";

            const int targetRes = static_cast<int>(detectedDanger);
            m_ResSwitchesRemaining = (targetRes - currentActualResType + MEDIGUN_NUM_RESISTS) % MEDIGUN_NUM_RESISTS;

            if (m_ResSwitchesRemaining > 0) {
                m_IsChangingRes = true;
                m_SwitchDelayTicks = 0;
                // Log that we are initiating a resistance switch.
                F::Output.ReportVaccinator(dangerSubject->entindex(), "switching", resStr, dangerReasonStr.c_str());
            }
            else { // Correct resistance is already active.
                if (shouldPopNow) {
                    // Pop immediately since we have the right resist.
                    pCmd->buttons |= IN_ATTACK2;
                    m_ShouldPop = false; // Popped, so no longer needed.
                    m_GoalResType = MEDIGUN_NUM_RESISTS; // Reset goal.
                    F::Output.ReportVaccinator(dangerSubject->entindex(), "popped", resStr, dangerReasonStr.c_str());
                    return; // Action taken, done for this tick.
                }
            }
        }
    }
}

void CAutoHeal::ProcessPlayerHurt(IGameEvent * event)
{
    const uint32_t uHash = FNV1A::Hash32(event->GetName());
    if (uHash != FNV1A::Hash32Const("player_hurt")) {
        return;
    }

    if (!Vars::Misc::Automation::AutoVaccinator_Active.Value)
        return;

    if (m_IsChangingRes || m_ShouldPop)
        return;

    const auto pLocal{ H::Entities.GetLocal() };
    if (!pLocal || pLocal->deadflag())
        return;

    const auto weapon{ H::Entities.GetWeapon() };
    if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
        return;

    const auto medigun{ weapon->As<CWeaponMedigun>() };
    if (!medigun || medigun->GetChargeType() < 3)
        return;

    const auto victim{ GET_ENT_FROM_USER_ID(event->GetInt("userid")) };
    const auto attacker{ GET_ENT_FROM_USER_ID(event->GetInt("attacker")) };

    if (!victim || victim == attacker)
        return;

    const auto health{ event->GetInt("health") };
    if (health <= 0)
        return;

    const auto weaponID{ event->GetInt("weaponid") };

    const CBaseEntity* currentHealTargetEntity = medigun->m_hHealingTarget().Get();
    if (victim != pLocal && victim != currentHealTargetEntity)
        return;

    const auto victimEnt{ victim->As<CTFPlayer>() };
    if (!victimEnt)
        return;

    const float percentHealth = Math::RemapValClamped(
        static_cast<float>(health), 0.0f, static_cast<float>(victimEnt->GetMaxHealth()), 0.0f, 1.0f
    );

    medigun_resist_types_t damageResType = WeaponIDToResType(weaponID);
    if (damageResType == MEDIGUN_NUM_RESISTS)
        return;

    m_GoalResType = damageResType;

    if (medigun->m_flChargeLevel() >= 0.25f && !PlayerHasResUber(damageResType, victimEnt))
    {
        bool popConditionMet = false;
        bool isCrit = event->GetBool("crit");
        bool isMiniCrit = event->GetBool("minicrit");

        if (isCrit || isMiniCrit || percentHealth < HEALTH_LIMIT) {
            popConditionMet = true;
        }

        if (damageResType == MEDIGUN_BLAST_RESIST && (isCrit || isMiniCrit || percentHealth < 1.0f)) {
            popConditionMet = true;
        }


        if (popConditionMet) {
            m_ShouldPop = true;
        }

        if (m_ShouldPop) {
            const char* resStr =
                damageResType == MEDIGUN_BULLET_RESIST ? "bullet" :
                damageResType == MEDIGUN_BLAST_RESIST ? "blast" :
                damageResType == MEDIGUN_FIRE_RESIST ? "fire" :
                "unknown";

            std::string reason;
            if (isCrit) reason = "crit";
            else if (isMiniCrit) reason = "mini-crit";
            else if (percentHealth < HEALTH_LIMIT) reason = "health too low";
            else reason = "other";

            F::Output.ReportVaccinator(victimEnt->entindex(), "popped", resStr, reason);
        }

    }

    if (m_ShouldPop && victim == currentHealTargetEntity) {
        bool isFriendOrParty = H::Entities.IsFriend(victimEnt->entindex()) || H::Entities.InParty(victimEnt->entindex());
        if (Vars::Aimbot::Healing::FriendsOnly.Value && !isFriendOrParty) {
            m_ShouldPop = false;
        }
    }
}

void CAutoHeal::PreventReload(CUserCmd * cmd)
{
    if (!Vars::Misc::Automation::AutoVaccinator_Active.Value)
        return;

    const auto pLocal = H::Entities.GetLocal();
    if (!pLocal || pLocal->deadflag()) return;

    const auto weapon{ H::Entities.GetWeapon() };
    if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
        return;

    const auto medigun{ weapon->As<CWeaponMedigun>() };
    if (!medigun || medigun->GetChargeType() < 3)
        return;

    if (!m_IsChangingRes && (cmd->buttons & IN_RELOAD))
    {
        cmd->buttons &= ~IN_RELOAD;
    }
}