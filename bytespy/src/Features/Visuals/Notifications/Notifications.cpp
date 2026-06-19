#include "Notifications.h"

static float EaseInOutCubic(float x)
{
    return x < 0.5f ? 4.f * x * x * x : 1 - powf(-2.f * x + 2.f, 3.f) / 2.f;
}

void CNotifications::Add(const std::string& sText, float flLifeTime, float flPanTime, const Color_t& tAccent, const Color_t& tBackground, const Color_t& tActive)
{
    m_vNotifications.emplace_back(sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime, tAccent, tBackground, tActive);
    while (m_vNotifications.size() > m_iMaxNotifySize)
        m_vNotifications.pop_front();
}

void CNotifications::Draw()
{
    for (auto it = m_vNotifications.begin(); it != m_vNotifications.end();)
    {
        if (it->m_flCreateTime + it->m_flLifeTime <= SDK::PlatFloatTime())
            it = m_vNotifications.erase(it);
        else
            ++it;
    }

    if (m_vNotifications.empty())
        return;

    const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
    const int iPadding = H::Draw.Scale(4);
    const int iLineHeight = H::Draw.Scale(2);
    const int iSpacing = H::Draw.Scale(4);

    const int iBaseX = H::Draw.Scale(8);
    int y = H::Draw.Scale(8);

    for (auto& tNotification : m_vNotifications)
    {
        const float flCurrentTime = SDK::PlatFloatTime();
        const float flElapsed = flCurrentTime - tNotification.m_flCreateTime;
        const float flRemaining = tNotification.m_flCreateTime + tNotification.m_flLifeTime - flCurrentTime;
        const float flRatio = std::clamp(flRemaining / tNotification.m_flLifeTime, 0.0f, 1.0f);

        const float flSlideInDuration = std::min(tNotification.m_flPanTime, tNotification.m_flLifeTime * 0.2f);
        const float flSlideOutDuration = std::min(tNotification.m_flPanTime, tNotification.m_flLifeTime * 0.2f);

        int x = iBaseX;

        if (flElapsed < flSlideInDuration)
        {
            float flProgress = EaseInOutCubic(flElapsed / flSlideInDuration);
            x -= static_cast<int>((1.f - flProgress) * H::Draw.Scale(200.f));
        }

        else if (flRemaining < flSlideOutDuration)
        {
            float flProgress = EaseInOutCubic(1.f - (flRemaining / flSlideOutDuration));
            x -= static_cast<int>(flProgress * H::Draw.Scale(200.f));
        }

        const std::string& sText = tNotification.m_sText;
        const size_t cTextSize = sText.size() + 1;
        wchar_t* wcText = new wchar_t[cTextSize];
        mbstowcs(wcText, sText.c_str(), cTextSize);

        int iTextWidth = 0, iTextHeight = 0;
        I::MatSystemSurface->GetTextSize(H::Fonts.GetFont(FONT_INDICATORS).m_dwFont, wcText, iTextWidth, iTextHeight);
        delete[] wcText;

        const int iBoxWidth = iTextWidth + 2 * iPadding;
        const int iBoxHeight = iTextHeight + 3 * iPadding + iLineHeight;

        H::Draw.FillRoundRect(x, y, iBoxWidth, iBoxHeight, H::Draw.Scale(4), { 0, 0, 0, 150 });

        H::Draw.StringOutlined(fFont, x + iPadding, y + iPadding,
            tNotification.m_tActive.Alpha(255), { 0, 0, 0, 150 }, ALIGN_TOPLEFT,
            sText.c_str());

        const int iLineY = y + iBoxHeight - iLineHeight - iPadding;
        const int iLineWidth = iBoxWidth - 2 * iPadding;
        const int iFilledWidth = static_cast<int>(iLineWidth * flRatio);

        H::Draw.FillRect(x + iPadding, iLineY, iLineWidth, iLineHeight, { 45, 45, 45, 200 });
        if (iFilledWidth > 0)
        {
            H::Draw.FillRect(x + iPadding, iLineY, iFilledWidth, iLineHeight,
                Vars::Menu::Theme::Accent.Value);
        }

        y += iBoxHeight + iSpacing;
    }
}
