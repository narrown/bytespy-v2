#include "SpectatorList.h"

#include "../../Players/PlayerUtils.h"
#include "../../Spectate/Spectate.h"
#include <unordered_set>   

bool CSpectatorList::GetSpectators(CTFPlayer* pTarget)
{
    m_vSpectators.clear();

    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        int iIndex = pPlayer->entindex();
        bool bLocal = pEntity->entindex() == I::EngineClient->GetLocalPlayer();

        auto pObserverTarget = pPlayer->m_hObserverTarget().Get();
        int iObserverMode = pPlayer->m_iObserverMode();
        if (bLocal && F::Spectate.m_iTarget != -1)
        {
            pObserverTarget = F::Spectate.m_pOriginalTarget;
            iObserverMode = F::Spectate.m_iOriginalMode;
        }

        if (pPlayer->IsAlive() || pObserverTarget != pTarget
            || bLocal && !I::EngineClient->IsPlayingDemo() && F::Spectate.m_iTarget == -1)
        {
            if (m_mRespawnCache.contains(pPlayer->entindex()))
                m_mRespawnCache.erase(pPlayer->entindex());
            continue;
        }

        std::string sMode;
        switch (iObserverMode)
        {
        case OBS_MODE_FIRSTPERSON: sMode = "1st"; break;
        case OBS_MODE_THIRDPERSON: sMode = "3rd"; break;
        default: continue;
        }

        int respawnIn = 0; float respawnTime = 0;
        if (auto pResource = H::Entities.GetPR())
        {
            respawnTime = pResource->GetNextRespawnTime(iIndex);
            respawnIn = std::max(respawnTime - I::GlobalVars->curtime, 0.f);
        }
        bool respawnTimeIncreased = false;
        if (!m_mRespawnCache.contains(iIndex))
            m_mRespawnCache[iIndex] = respawnTime;
        if (m_mRespawnCache[iIndex] + 0.9f < respawnTime)
        {
            respawnTimeIncreased = true;
            m_mRespawnCache[iIndex] = -1.f;
        }

        PlayerInfo_t pi{};
        if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
        {
            std::string sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);

            m_vSpectators.emplace_back(
                sName,
                sMode,
                respawnIn,
                respawnTimeIncreased,
                H::Entities.IsFriend(iIndex),
                H::Entities.InParty(iIndex),
                iIndex
            );

            m_vSpectators.emplace_back(sName, sMode, respawnIn, respawnTimeIncreased, H::Entities.IsFriend(pPlayer->entindex()), H::Entities.InParty(pPlayer->entindex()), pPlayer->entindex());

        }
    }

    return !m_vSpectators.empty();
}

void CSpectatorList::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Spectators))
	{
		m_mRespawnCache.clear();
		return;
	}

	auto pTarget = pLocal;
	switch (pLocal->m_iObserverMode())
	{
	case OBS_MODE_FIRSTPERSON:
	case OBS_MODE_THIRDPERSON:
		pTarget = pLocal->m_hObserverTarget().Get()->As<CTFPlayer>();
	}

	PlayerInfo_t pi{};
	if (!pTarget || (pTarget != pLocal && !I::EngineClient->GetPlayerInfo(pTarget->entindex(), &pi)))
		return;

	m_vSpectators.clear();
	GetSpectators(pTarget);

	// remove duplicates
	{
		std::unordered_set<int> seen;
		auto it = m_vSpectators.begin();
		while (it != m_vSpectators.end())
		{
			if (!seen.insert(it->m_iIndex).second)
				it = m_vSpectators.erase(it);
			else
				++it;
		}
	}

	const DragBox_t dtPos = Vars::Menu::SpectatorsDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);
	const int iPadding = H::Draw.Scale(2);
	const int iLineH = H::Draw.Scale(2);
	const int avgCW = H::Draw.Scale(7);
	const int iMinW = H::Draw.Scale(150);
	const Color_t lineColor = Vars::Menu::Theme::Accent.Value;

	std::string sName = (pTarget != pLocal)
		? F::PlayerUtils.GetPlayerName(pTarget->entindex(), pi.name)
		: "you";
	std::string headerText = (pTarget == pLocal)
		? "spectator list"
		: std::format("spectating {}", sName);

	size_t maxLen = headerText.length();
	for (auto& sp : m_vSpectators)
	{
		std::string entry = std::format("{} - {} (respawn {}s)",
			sp.m_sName, sp.m_sMode, sp.m_iRespawnIn);
		maxLen = std::max(maxLen, entry.length());
	}
	int textW = static_cast<int>(maxLen * avgCW);

	bool hasSpecs = !m_vSpectators.empty();
	int iWidth = hasSpecs
		? std::max(textW + 2 * iPadding, iMinW)
		: (textW + 2 * iPadding);

	int iContentH = iPadding + nTall + iPadding + iLineH + iPadding;
	if (hasSpecs)
		iContentH += nTall * m_vSpectators.size();

	int iPosX = dtPos.x - iWidth / 2;
	int iPosY = dtPos.y;
	int iRadius = H::Draw.Scale(4);

	// Conditional background
	if (!Vars::Menu::RemoveBackground.Value)
	{
		H::Draw.FillRoundRect(iPosX, iPosY, iWidth, iContentH, iRadius, { 0, 0, 0, 150 });
	}

	// Header
	H::Draw.StringOutlined(
		fFont,
		iPosX + iPadding,
		iPosY + iPadding,
		Vars::Menu::Theme::Accent.Value,
		{ 0, 0, 0, 150 },
		ALIGN_TOPLEFT,
		headerText.c_str()
	);

	// Separator line
	int lineY = iPosY + iPadding + nTall + (iPadding / 2);
	H::Draw.FillRect(iPosX + iPadding, lineY, iWidth - 2 * iPadding, iLineH, lineColor);

	// Spectator entries
	if (hasSpecs)
	{
		int yOff = lineY + iLineH + (iPadding / 2);
		for (auto& sp : m_vSpectators)
		{
			Color_t color = Vars::Menu::Theme::Active.Value;
			if (sp.m_bIsFriend)
				color = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)].Color;
			else if (sp.m_bInParty)
				color = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].Color;
			else if (sp.m_bRespawnTimeIncreased)
				color = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(CHEATER_TAG)].Color;
			else if (FNV1A::Hash32(sp.m_sMode.c_str()) == FNV1A::Hash32Const("1st"))
				color = color.Lerp({ 255, 150, 0, 255 }, 0.5f);

			H::Draw.StringOutlined(
				fFont,
				iPosX + iPadding,
				yOff,
				color,
				{ 0, 0, 0, 150 },
				ALIGN_TOPLEFT,
				std::format("{} - {} (respawn {}s)",
					sp.m_sName, sp.m_sMode, sp.m_iRespawnIn).c_str()
			);
			yOff += nTall;
		}
	}
}


