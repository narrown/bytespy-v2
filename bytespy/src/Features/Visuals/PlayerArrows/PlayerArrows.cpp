#include "PlayerArrows.h"

void CPlayerArrows::DrawArrowTo(const Vec3& vFromPos, const Vec3& vToPos, Color_t tColor)
{
    const float flMaxDistance = Vars::Visuals::FOVArrows::MaxDist.Value;
    const float flMap = Math::RemapVal(vFromPos.DistTo(vToPos), flMaxDistance, flMaxDistance * 0.9f, 0.0f, 1.0f);
    tColor.a = static_cast<byte>(flMap * 255.0f);
    if (!tColor.a)
        return;

    Vec2 vCenter = { H::Draw.m_nScreenW * 0.5f, H::Draw.m_nScreenH * 0.5f };
    Vec3 vScreenPos;
    bool bOnScreen = SDK::W2S(vToPos, vScreenPos, true);

    if (bOnScreen)
    {
        float flMin = std::min(vCenter.x, vCenter.y);
        float flMax = std::max(vCenter.x, vCenter.y);
        float flDist = std::hypot(vScreenPos.x - vCenter.x, vScreenPos.y - vCenter.y);
        float flTransparency = 1.0f - std::clamp((flDist - flMin) / (flMax - flMin), 0.0f, 1.0f);
        tColor.a = static_cast<byte>(std::max(float(tColor.a) - flTransparency * 255.0f, 0.0f));
        if (!tColor.a)
            return;
    }

    if (Vars::Visuals::FOVArrows::OOBStyle.Value == Vars::Visuals::FOVArrows::OOBStyleEnum::Diamond)
    {
        Vec2 vDir = { vScreenPos.x - vCenter.x, vScreenPos.y - vCenter.y };
        float flAngle = std::atan2(vDir.y, vDir.x);
        float flOffset = Vars::Visuals::FOVArrows::Offset.Value;
        float flSize = H::Draw.Scale(30);
        float halfW = flSize * 0.35f;
        float tailLen = flSize * 0.4f;
        float tailTipW = flSize * 0.15f;

        Vec2 pts[6] = {
            { flOffset + flSize, 0.0f },
            { flOffset, halfW },
            { flOffset - tailLen, tailTipW },
            { flOffset - tailLen * 1.1f, 0.0f },
            { flOffset - tailLen, -tailTipW },
            { flOffset, -halfW }
        };

        Vec2 out[6];
        for (int i = 0; i < 6; ++i)
        {
            float x = pts[i].x * cosf(flAngle) - pts[i].y * sinf(flAngle);
            float y = pts[i].x * sinf(flAngle) + pts[i].y * cosf(flAngle);
            out[i] = { vCenter.x + x, vCenter.y + y };
        }

        H::Draw.FillPolygon({
            { { out[0].x, out[0].y } },
            { { out[1].x, out[1].y } },
            { { out[2].x, out[2].y } },
            { { out[3].x, out[3].y } },
            { { out[4].x, out[4].y } },
            { { out[5].x, out[5].y } }
            }, tColor);
        return;
    }
    else if (Vars::Visuals::FOVArrows::OOBStyle.Value == Vars::Visuals::FOVArrows::OOBStyleEnum::Arrow)
    {
        const float dx = vScreenPos.x - vCenter.x;
        const float dy = vScreenPos.y - vCenter.y;
        const float flDeg = atan2(dy, dx);
        const float flCos = cos(flDeg);
        const float flSin = sin(flDeg);

        float flOffset = Vars::Visuals::FOVArrows::Offset.Value;
        float flScale = H::Draw.Scale(25);

        const float x1 = flOffset * flCos;
        const float y1 = flOffset * flSin;
        const float x2 = (flOffset + 10.f) * flCos;
        const float y2 = (flOffset + 10.f) * flSin;

        const float arrow_angle = DEG2RAD(90.f);
        const float arrow_length = flScale;

        const Vec2 line = { x2 - x1, y2 - y1 };
        const float length = sqrt(line.x * line.x + line.y * line.y);

        const float f_point_on_line = arrow_length / (atanf(arrow_angle) * length);
        const Vec2 point_on_line = { x2 + line.x * f_point_on_line * -1.0f, y2 + line.y * f_point_on_line * -1.0f };
        const Vec2 normal_vector = { -line.y, line.x };
        const Vec2 normal = { arrow_length / (length * 2), arrow_length / (length * 2) };

        const Vec2 rotation = { normal.x * normal_vector.x, normal.y * normal_vector.y };
        const Vec2 left = { point_on_line.x + rotation.x, point_on_line.y + rotation.y };
        const Vec2 right = { point_on_line.x - rotation.x, point_on_line.y - rotation.y };

        H::Draw.Line(vCenter.x + left.x, vCenter.y + left.y, vCenter.x + x2, vCenter.y + y2, tColor);
        H::Draw.Line(vCenter.x + right.x, vCenter.y + right.y, vCenter.x + x2, vCenter.y + y2, tColor);

        H::Draw.Line(vCenter.x + left.x, vCenter.y + left.y,
            vCenter.x + (point_on_line.x + (x2 - point_on_line.x) * 0.45f),
            vCenter.y + (point_on_line.y + (y2 - point_on_line.y) * 0.45f), tColor);

        H::Draw.Line(vCenter.x + right.x, vCenter.y + right.y,
            vCenter.x + (point_on_line.x + (x2 - point_on_line.x) * 0.45f),
            vCenter.y + (point_on_line.y + (y2 - point_on_line.y) * 0.45f), tColor);

        Color_t tColorFaded = tColor;
        tColorFaded.a = 60;
        H::Draw.FillPolygon(
            {
                { { vCenter.x + x2, vCenter.y + y2 } },
                { { vCenter.x + left.x, vCenter.y + left.y } },
                { { vCenter.x + (point_on_line.x + (x2 - point_on_line.x) * 0.45f),
                    vCenter.y + (point_on_line.y + (y2 - point_on_line.y) * 0.45f) } }
            }, tColorFaded
        );

        H::Draw.FillPolygon(
            {
                { { vCenter.x + x2, vCenter.y + y2 } },
                { { vCenter.x + right.x, vCenter.y + right.y } },
                { { vCenter.x + (point_on_line.x + (x2 - point_on_line.x) * 0.45f),
                    vCenter.y + (point_on_line.y + (y2 - point_on_line.y) * 0.45f) } }
            }, tColorFaded
        );
    }
}

void CPlayerArrows::Run(CTFPlayer* pLocal)
{
    if (!Vars::Visuals::FOVArrows::Enabled.Value)
        return;

    Vec3 vLocalPos = pLocal->GetEyePosition();
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if ((pPlayer->IsDormant() && !H::Entities.GetDormancy(pPlayer->entindex())) ||
            !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
            continue;

        Color_t tColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
        if (pPlayer->InCond(TF_COND_DISGUISED))
            tColor = Vars::Colors::Target.Value;

        DrawArrowTo(vLocalPos, pPlayer->GetCenter(), tColor);
    }
}
