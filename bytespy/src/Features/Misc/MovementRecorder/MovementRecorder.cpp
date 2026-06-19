#include "MovementRecorder.h"

void CMovementRecorder::StartRecording()
{
    m_bIsRecording = true;
    m_bIsPlaying = false;
    m_vFrames.clear();
    m_flRecordStart = I::GlobalVars->curtime;
    m_iRecordedTickCounter = 0;

    auto pLocalEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    if (pLocalEntity)
        m_vStartPosition = pLocalEntity->GetAbsOrigin();
}

void CMovementRecorder::StopRecording()
{
    m_bIsRecording = false;
}

void CMovementRecorder::StartPlayback(CUserCmd* pCmd)
{
    if (m_vFrames.empty())
        return;

    m_bIsPlaying = true;
    m_bIsRecording = false;
    m_bHomingToStart = true;
    m_iPlaybackFrame = 0;
    m_iPlaybackTick = I::GlobalVars->tickcount;
}

void CMovementRecorder::StopPlayback()
{
    m_bIsPlaying = false;
    m_bHomingToStart = false;
    m_iPlaybackFrame = 0;
}

void CMovementRecorder::CreateMove(CUserCmd* pCmd)
{
    if (!Vars::MovementRecorder::Enabled.Value)
        return;

    const auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;

    Vec3 vCurrent = pLocal->GetAbsOrigin();

    if (m_bHomingToStart)
    {
        float dist = (vCurrent - m_vStartPosition).LengthSqr();
        constexpr float eps = 0.9f;

        if (dist > eps * eps)
        {
            trace_t tr;
            Ray_t ray;
            ray.Init(pLocal->GetEyePosition(), m_vStartPosition);

            CTraceFilterSkipLocal filter(pLocal);
            I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

            if (tr.fraction >= 0.99f)
            {
                SDK::WalkTo(pCmd, pLocal, vCurrent, m_vStartPosition, 1.0f);
                return;
            }

            return;
        }

        m_bHomingToStart = false;
        m_bIsPlaying = true;
        m_iPlaybackFrame = 0;
        m_iPlaybackTick = I::GlobalVars->tickcount;
    }


    static bool bPrevSaveKey = false;
    static bool bPrevPlaybackKey = false;
    bool bSaveKey = Vars::MovementRecorder::Save.Value;
    bool bPlaybackKey = Vars::MovementRecorder::Playback.Value;

    if (bSaveKey && !bPrevSaveKey)           StartRecording();
    if (!bSaveKey && bPrevSaveKey)           StopRecording();

    if (bPlaybackKey && !bPrevPlaybackKey)   StartPlayback(pCmd);
    if (!bPlaybackKey && bPrevPlaybackKey)   StopPlayback();

    bPrevSaveKey = bSaveKey;
    bPrevPlaybackKey = bPlaybackKey;

    if (m_bIsRecording)
    {
        RecordedFrame_t frame{};
        frame.vPosition = vCurrent;
        frame.viewAngles = pCmd->viewangles;
        frame.forwardMove = pCmd->forwardmove;
        frame.sideMove = pCmd->sidemove;
        frame.upMove = pCmd->upmove;
        frame.buttons = pCmd->buttons;
        frame.iRelativeTick = m_iRecordedTickCounter++;
        m_vFrames.push_back(frame);
    }

    if (m_bIsPlaying && m_iPlaybackTick >= 0)
    {
        int tickOffset = I::GlobalVars->tickcount - m_iPlaybackTick;
        if (tickOffset < 0)
            tickOffset = 0;

        while (m_iPlaybackFrame + 1 < m_vFrames.size() &&
            m_vFrames[m_iPlaybackFrame + 1].iRelativeTick <= tickOffset)
        {
            ++m_iPlaybackFrame;
        }

        if (m_iPlaybackFrame >= m_vFrames.size())
        {
            StopPlayback();
            return;
        }

        const auto& frame = m_vFrames[m_iPlaybackFrame];
        pCmd->viewangles = frame.viewAngles;
        pCmd->forwardmove = frame.forwardMove;
        pCmd->sidemove = frame.sideMove;
        pCmd->upmove = frame.upMove;
        pCmd->buttons = frame.buttons;
    }
}

void CMovementRecorder::EngineVGui()
{
    if (!Vars::MovementRecorder::Enabled.Value)
        return;

    if (Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::MovementRecorder)
    {
        const int iCurrentTick = m_iPlaybackFrame;
        const int iTotalTicks = static_cast<int>(m_vFrames.size());
        const float flProgress = iTotalTicks > 0
            ? static_cast<float>(iCurrentTick) / static_cast<float>(iTotalTicks)
            : 0.f;

        const DragBox_t dtPos = Vars::Menu::MovementRecorderDisplay.Value;
        const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
        const int iWidth = H::Draw.Scale(150);
        const int iHeight = H::Draw.Scale(25);
        const int iLineHeight = H::Draw.Scale(2);
        const int iPadding = H::Draw.Scale(4);

        const int iPosX = dtPos.x - iWidth / 2;
        const int iPosY = dtPos.y;

        if (!Vars::Menu::RemoveBackground.Value)
        {
            H::Draw.FillRoundRect(
                iPosX, iPosY, iWidth, iHeight,
                H::Draw.Scale(4), { 0, 0, 0, 150 }
            );
        }

        H::Draw.StringOutlined(
            fFont,
            iPosX + iPadding,
            iPosY + iPadding,
            Vars::Menu::Theme::Active.Value,
            { 0, 0, 0, 150 },
            ALIGN_TOPLEFT,
            std::format("tick {}", iCurrentTick).c_str()
        );

        H::Draw.StringOutlined(
            fFont,
            iPosX + iWidth - iPadding,
            iPosY + iPadding,
            Vars::Colors::IndicatorTextMisc.Value,
            { 0, 0, 0, 150 },
            ALIGN_TOPRIGHT,
            std::format("max: {}", iTotalTicks).c_str()
        );

        const int iBarY = iPosY + iHeight - iLineHeight - iPadding;
        static float flDisplayedProgress = 0.0f;
        {
            constexpr float SMOOTH_SPEED = 15.0f;
            const float delta = I::GlobalVars->frametime * SMOOTH_SPEED;
            flDisplayedProgress = std::lerp(
                flDisplayedProgress,
                flProgress,
                std::clamp(delta, 0.0f, 1.0f)
            );
        }
        const int iBarWidth = static_cast<int>(
            (iWidth - iPadding * 2) * flDisplayedProgress
            );

        H::Draw.FillRect(
            iPosX + iPadding, iBarY,
            iWidth - iPadding * 2, iLineHeight,
            { 45, 45, 45, 200 }
        );

        if (iBarWidth > 0)
        {
            H::Draw.FillRect(
                iPosX + iPadding, iBarY,
                iBarWidth, iLineHeight,
                Vars::Menu::Theme::Accent.Value
            );
        }
    }

    Vec3 screenPos;
    if (!m_bIsPlaying && Vars::MovementRecorder::VisualizeSpot.Value)
    {
        constexpr int segments = 32;
        constexpr float radiusWS = 20.0f;   
        Color_t outlineColor = Vars::MovementRecorder::VisualizeSpotColor.Value;
        Color_t fillColor = outlineColor;
        fillColor.a = 100;       

        std::vector<Vertex_t> verts;
        verts.reserve(segments);

        for (int i = 0; i < segments; ++i)
        {
            float angle = DEG2RAD(360.0f / segments * i);
            Vec3 worldPt = m_vStartPosition
                + Vec3{ std::cos(angle) * radiusWS,
                        std::sin(angle) * radiusWS,
                        0.0f };
            Vec3 scrnPt;
            if (!SDK::W2S(worldPt, scrnPt, false))
                return;   

            Vertex_t vtx;
            vtx.m_Position.x = scrnPt.x;
            vtx.m_Position.y = scrnPt.y;
            verts.push_back(vtx);
        }

        H::Draw.FillPolygon(verts, fillColor);

        for (int i = 0; i < segments; ++i)
        {
            const auto& p1 = verts[i].m_Position;
            const auto& p2 = verts[(i + 1) % segments].m_Position;

            H::Draw.Line(
                static_cast<int>(p1.x), static_cast<int>(p1.y),
                static_cast<int>(p2.x), static_cast<int>(p2.y),
                outlineColor
            );
        }
    }


    if (!m_vFrames.empty() &&
        (m_bIsPlaying && !m_bIsRecording) &&
        Vars::MovementRecorder::Line.Value)
    {
        for (size_t i = 1; i < m_vFrames.size(); ++i)
        {
            const Vec3& pos1 = m_vFrames[i - 1].vPosition;
            const Vec3& pos2 = m_vFrames[i].vPosition;

            Vec3 screen1, screen2;
            if (SDK::W2S(pos1, screen1, false) &&
                SDK::W2S(pos2, screen2, false))
            {
                H::Draw.Line(
                    static_cast<int>(screen1.x),
                    static_cast<int>(screen1.y),
                    static_cast<int>(screen2.x),
                    static_cast<int>(screen2.y),
                    Vars::MovementRecorder::LineColor.Value
                );
            }
        }
    }
}
