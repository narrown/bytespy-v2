#include "Menu.h"

#include "Components.h"
#include "../../Configs/Configs.h"
#include "../../Binds/Binds.h"
#include "../../Players/PlayerUtils.h"
#include "../../CameraWindow/CameraWindow.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"
#include "../../Resolver/Resolver.h"
#include "../../Misc/Misc.h"
#include "../../Output/Output.h"
#include <windows.h>
#include <shellapi.h>
#include "../../Spectate/Spectate.h"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

inline std::string Base64Encode(const std::string& input)
{
	std::string output;
	int val = 0, valb = -6;
	for (uint8_t c : input)
	{
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0)
		{
			output.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
	while (output.size() % 4) output.push_back('=');
	return output;
}

inline std::string Base64Decode(const std::string& input)
{
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

	std::string output;
	int val = 0, valb = -8;
	for (uint8_t c : input)
	{
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0)
		{
			output.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return output;
}

// Animation state
struct TabAnimation {
	float MainTabAlpha = 0.f;
	float SubTabAlpha = 0.f;
	int PreviousMainTab = 0;
	int PreviousSubTab = 0;
	float MainTabWidth = 0.f;
	float SubTabWidth = 0.f;
	float MainTabOffset = 0.f;
	float SubTabOffset = 0.f;
};

static TabAnimation s_TabAnimation;

void CMenu::DrawMenu()
{
	using namespace ImGui;

	const float BORDER_SIZE = 2.0f;
	const ImVec2 vBorderPadding(BORDER_SIZE, BORDER_SIZE);
	const float ANIM_SPEED = 15.f * GetIO().DeltaTime;
	const ImU32 accentColor = ImGui::ColorConvertFloat4ToU32(F::Render.Accent.Value);

	static ImVec2 g_vMenuPos = ImVec2(100, 100);
	static ImVec2 g_vMenuTargetPos = g_vMenuPos;
	static bool g_bDragging = false;
	static ImVec2 g_vDragOffset;

	ImVec2 mousePos = GetIO().MousePos;
	bool leftDown = IsMouseDown(0);
	bool leftClicked = IsMouseClicked(0);

	// Apply current position
	g_vMenuPos.x = ImLerp(g_vMenuPos.x, g_vMenuTargetPos.x, ImClamp(ANIM_SPEED, 0.f, 1.f));
	g_vMenuPos.y = ImLerp(g_vMenuPos.y, g_vMenuTargetPos.y, ImClamp(ANIM_SPEED, 0.f, 1.f));

	SetNextWindowPos(g_vMenuPos, ImGuiCond_Always);
	SetNextWindowSize({ 600, 500 }, ImGuiCond_FirstUseEver);
	PushStyleVar(ImGuiStyleVar_WindowPadding, vBorderPadding);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(600), H::Draw.Scale(500) });
	PushStyleVar(ImGuiStyleVar_ChildRounding, H::Draw.Scale(0));
	PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));

	if (Begin("main", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize))
	{
		// Handle background dragging
		if (IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_RootAndChildWindows)
			&& !IsAnyItemHovered() && leftClicked)
		{
			g_bDragging = true;
			g_vDragOffset = mousePos - g_vMenuPos;
		}

		if (!leftDown)
			g_bDragging = false;

		if (g_bDragging)
			g_vMenuTargetPos = mousePos - g_vDragOffset;

		const ImVec2 vWindowSize = GetWindowSize();
		const ImVec2 vInnerSize = {
			vWindowSize.x - (vBorderPadding.x * 2),
			vWindowSize.y - (vBorderPadding.y * 2)
		};

		static int iMainTab = 0;
		static std::array<int, 6> iSubTabs = { 0 };
		const float flMainTabHeight = H::Draw.Scale(40);
		const float flSubTabHeight = H::Draw.Scale(30);

		if (s_TabAnimation.PreviousMainTab != iMainTab) {
			s_TabAnimation.PreviousMainTab = iMainTab;
			s_TabAnimation.MainTabAlpha = 0.f;
		}
		else {
			s_TabAnimation.MainTabAlpha = ImMin(s_TabAnimation.MainTabAlpha + ANIM_SPEED, 1.f);
		}

		if (s_TabAnimation.PreviousSubTab != iSubTabs[iMainTab]) {
			s_TabAnimation.PreviousSubTab = iSubTabs[iMainTab];
			s_TabAnimation.SubTabAlpha = 0.f;
		}
		else {
			s_TabAnimation.SubTabAlpha = ImMin(s_TabAnimation.SubTabAlpha + ANIM_SPEED, 1.f);
		}

		PushFont(F::Render.FontLarge);
		std::vector<const char*> vIcons = {};
		PushFont(F::Render.FontLarge);

		{
			PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 255));
			PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 255));
			PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 255));

			const float tabWidth = vInnerSize.x / 6;
			FTabs(
				{
					{ "aimbot", "hvh", "automations", "visuals", "misc", "configs" }
				},
				{ &iMainTab },
				{ tabWidth, flMainTabHeight },
				{ vBorderPadding.x, vBorderPadding.y },
				FTabs_Horizontal | FTabs_AlignCenter,
				vIcons,
				{ vIcons.empty() ? 0.0f : H::Draw.Scale(10), 0 }
			);

			PopStyleColor(3);

			if (s_TabAnimation.MainTabWidth == 0.f) {
				s_TabAnimation.MainTabWidth = tabWidth;
				s_TabAnimation.MainTabOffset = iMainTab * tabWidth;
			}
			else {
				s_TabAnimation.MainTabWidth = ImLerp(s_TabAnimation.MainTabWidth, tabWidth, ANIM_SPEED);
				s_TabAnimation.MainTabOffset = ImLerp(s_TabAnimation.MainTabOffset, iMainTab * tabWidth, ANIM_SPEED);
			}

			ImDrawList* drawList = GetWindowDrawList();
			const ImVec2 windowPos = GetWindowPos();
			const float underlineY = windowPos.y + vBorderPadding.y + flMainTabHeight - H::Draw.Scale(2);

			drawList->AddRectFilled(
				{ windowPos.x + vBorderPadding.x + s_TabAnimation.MainTabOffset, underlineY },
				{ windowPos.x + vBorderPadding.x + s_TabAnimation.MainTabOffset + s_TabAnimation.MainTabWidth, underlineY + H::Draw.Scale(2) },
				accentColor
			);
		}

		PopFont();
		PopFont();

		const std::vector<std::vector<const char*>> vSubItems = {
			{ "general" },
			{ "main##" },
			{ "main###" },
			{ "esp", "chams", "glow", "misc##", "logs##", "menu" },
			{ "main", "movement recorder"},
			{ "config", "playerlist" }
		};

		PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 255));
		PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 255));
		PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 255));

		if (!vSubItems[iMainTab].empty())
		{
			const float subTabWidth = vInnerSize.x / vSubItems[iMainTab].size();
			FTabs(
				vSubItems[iMainTab],
				&iSubTabs[iMainTab],
				{ subTabWidth, flSubTabHeight },
				{ vBorderPadding.x, flMainTabHeight + vBorderPadding.y },
				FTabs_Horizontal | FTabs_AlignCenter
			);

			if (s_TabAnimation.SubTabWidth == 0.f) {
				s_TabAnimation.SubTabWidth = subTabWidth;
				s_TabAnimation.SubTabOffset = iSubTabs[iMainTab] * subTabWidth;
			}
			else {
				s_TabAnimation.SubTabWidth = ImLerp(s_TabAnimation.SubTabWidth, subTabWidth, ANIM_SPEED);
				s_TabAnimation.SubTabOffset = ImLerp(s_TabAnimation.SubTabOffset, iSubTabs[iMainTab] * subTabWidth, ANIM_SPEED);
			}

			ImDrawList* drawList = GetWindowDrawList();
			const ImVec2 windowPos = GetWindowPos();
			const float underlineY = windowPos.y + vBorderPadding.y + flMainTabHeight + flSubTabHeight - H::Draw.Scale(2);

			drawList->AddRectFilled(
				{ windowPos.x + vBorderPadding.x + s_TabAnimation.SubTabOffset, underlineY },
				{ windowPos.x + vBorderPadding.x + s_TabAnimation.SubTabOffset + s_TabAnimation.SubTabWidth, underlineY + H::Draw.Scale(2) },
				accentColor
			);
		}

		PopStyleColor(3);

		const ImVec2 contentSize = {
			vInnerSize.x,
			vInnerSize.y - flMainTabHeight - flSubTabHeight
		};

		SetCursorPos({
			vBorderPadding.x,
			vBorderPadding.y + flMainTabHeight + flSubTabHeight
			});

		PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
		PushStyleVar(ImGuiStyleVar_Alpha, s_TabAnimation.MainTabAlpha * s_TabAnimation.SubTabAlpha);

		if (BeginChild("content", contentSize, ImGuiChildFlags_AlwaysUseWindowPadding))
		{
			switch (iMainTab)
			{
			case 0: MenuAimbot(iSubTabs[0]); break;
			case 1: MenuHvH(iSubTabs[1]); break;
			case 2: MenuAutomations(iSubTabs[2]); break;
			case 3: MenuVisuals(iSubTabs[3]); break;
			case 4: MenuMisc(iSubTabs[4]); break;
			case 5: MenuSettings(iSubTabs[5]); break;
			default: break;
			}
		}
		EndChild();
		PopStyleVar(2);

		Bind_t tBind;
		if (F::Binds.GetBind(CurrentBind, &tBind) && CurrentBind != DEFAULT_BIND)
		{
			const float overlayWidth = H::Draw.Scale(250);
			const float overlayHeight = H::Draw.Scale(30);
			const ImVec2 overlaySize = { overlayWidth, overlayHeight };

			const ImVec2 windowPos = GetWindowPos();
			const ImVec2 overlayPos = {
				windowPos.x + (GetWindowSize().x - overlayWidth) * 0.5f,
				windowPos.y - overlayHeight - H::Draw.Scale(8)
			};

			SetNextWindowPos(overlayPos, ImGuiCond_Always);
			SetNextWindowSize(overlaySize, ImGuiCond_Always);
			PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
			PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));

			Begin("##BindOverlay", nullptr,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoBackground);

			ImDrawList* drawList = GetWindowDrawList();
			ImVec2 pos = GetWindowPos();

			drawList->AddRectFilled(
				pos,
				pos + overlaySize,
				IM_COL32(7, 7, 7, 255)
			);
			drawList->AddRect(
				pos,
				pos + overlaySize,
				accentColor,
				0.0f,
				0,
				BORDER_SIZE
			);

			SetCursorPos({ H::Draw.Scale(10), (overlayHeight - GetTextLineHeight()) * 0.5f });
			PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
			Text("how... how are you seeing this?", tBind.m_sName);
			PopStyleColor();

			SetCursorPos({ overlayWidth - H::Draw.Scale(30), (overlayHeight - H::Draw.Scale(20)) * 0.5f });
			PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 2 });
			PushStyleColor(ImGuiCol_Button, IM_COL32(30, 30, 30, 255));
			PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(50, 50, 50, 255));
			PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(70, 70, 70, 255));
			if (Button("X"))
				CurrentBind = DEFAULT_BIND;
			PopStyleColor(3);
			PopStyleVar();

			End();
			PopStyleColor();
			PopStyleVar();
		}

		ImDrawList* drawList = GetWindowDrawList();
		const ImVec2 windowPos = GetWindowPos();
		drawList->AddRect(
			windowPos,
			windowPos + vWindowSize,
			accentColor,
			0.0f,
			0,
			BORDER_SIZE
		);

		End();
	}
	PopStyleVar(3);
	PopStyleColor();
}


#pragma region Tabs
void CMenu::MenuAimbot(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
		// General
	case 0:
		if (BeginTable("AimbotTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("general"))
			{
				FDropdown("aimbot", Vars::Aimbot::General::AimType, { "off", "plain", "smooth", "silent", "lock", "aim assist" }, {}, FDropdown_Left);
				FDropdown("target selection", Vars::Aimbot::General::TargetSelection, { "FOV", "distance" }, {}, FDropdown_Right);
				FDropdown("target", Vars::Aimbot::General::Target, { "players", "sentries", "dispensers", "teleporters", "stickies", "NPC's", "bombs" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("ignore", Vars::Aimbot::General::Ignore, { "steam friends", "party", "invulnerable", "cloaked", "unsimulated players", "feign", "vaccinator", "disguised", "taunting" }, {}, FDropdown_Right | FDropdown_Multi);
				FSlider("aimbot FOV", Vars::Aimbot::General::AimFOV, 0.f, 180.f, 1.f, "%g", FSlider_Clamp | FSlider_Precision);
				FSlider("max targets", Vars::Aimbot::General::MaxTargets, 1, 6, 1, "%i", FSlider_Left | FSlider_Min);
				PushTransparent(FGet(Vars::Aimbot::General::AimType) != Vars::Aimbot::General::AimTypeEnum::Smooth && FGet(Vars::Aimbot::General::AimType) != Vars::Aimbot::General::AimTypeEnum::Assistive);
				{
					FSlider("smooth sens", Vars::Aimbot::General::AssistStrength, 0.f, 100.f, 1.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Cloaked));
				{
					FSlider("ignore cloaked", Vars::Aimbot::General::IgnoreCloakPercentage, 0, 100, 10, "%d%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Unsimulated));
				{
					FSlider("tick tolerance", Vars::Aimbot::General::TickTolerance, 0, 21, 1, "%i", FSlider_Right | FSlider_Clamp);
				}
				PopTransparent();
				FColorPicker("FOV circle", Vars::Colors::FOVCircle, 0, FColorPicker_None, nullptr, "FOV range color");
				FToggle("autoshoot", Vars::Aimbot::General::AutoShoot, FToggle_Left);
				FToggle("FOV circle", Vars::Aimbot::General::FOVCircle, FToggle_Right);
				FToggle("crithack", Vars::CritHack::ForceCrits, FToggle_Left);
				FToggle("no random crits", Vars::CritHack::AvoidRandom, FToggle_Right);
				FToggle("always crit on melee", Vars::CritHack::AlwaysMeleeCrit, FToggle_Left);
				FToggle("seed prediction", Vars::Aimbot::General::NoSpread, FToggle_Right);
				FToggle("auto secondary switch", Vars::Aimbot::General::AutoSecondarySwitch, FToggle_Left);
			} EndSection();

			if (Section("melee", true))
			{
				FToggle("auto backstab", Vars::Aimbot::Melee::AutoBackstab, FToggle_Left);
				FToggle("ignore razorback", Vars::Aimbot::Melee::IgnoreRazorback, FToggle_Right);
				FToggle("swing prediction", Vars::Aimbot::Melee::SwingPrediction, FToggle_Left);
				FToggle("swing predict lag", Vars::Aimbot::Melee::SwingPredictLag, FToggle_Right);
				//				FToggle("Whip teammates", Vars::Aimbot::Melee::WhipTeam, FToggle_Right);
			} EndSection();

			if (Section("beta settings", true))
			{
				FToggle("high arc", Vars::Aimbot::Projectile::HighArc, FToggle_Left);
				FToggle("unstable high arc", Vars::Aimbot::Projectile::UnstableHighArc, FToggle_Right);
				FTooltip("shoots every possible spot but will decrease accuracy");
				FToggle("roll angles", Vars::Aimbot::Projectile::RollAngles, FToggle_Left);
				FSlider("roll angleZ", Vars::Aimbot::Projectile::RollAngleZ, 20.f, 180.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
			} EndSection();

			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Backtrack", true))
				{
					FSlider("Offset", Vars::Backtrack::Offset, -1, 1);
				} EndSection();
			}

			/* Column 2 */
			TableNextColumn();
			if (Section("hitscan"))
			{
				FDropdown("hitboxes## hitscan", Vars::Aimbot::Hitscan::Hitboxes, { "head", "body", "pelvis", "arms", "legs", "bodyaim if lethal" }, {}, FDropdown_Left | FDropdown_Multi, 0, nullptr, "hitscan hitboxes");
				FDropdown("priority hitboxes##", Vars::Aimbot::Hitscan::StaticHitboxes, { "head", "body", "pelvis" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "priority hitscan hitboxes");
				FDropdown("modifiers## hitscan", Vars::Aimbot::Hitscan::Modifiers, { "tapfire", "wait for headshot", "wait for charge", "scoped only", "auto scope", "auto rev minigun", "extinguish team" }, {}, FDropdown_None | FDropdown_Multi, 0, nullptr, "hitscan modifiers");
				FSlider("point scale", Vars::Aimbot::Hitscan::PointScale, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PushTransparent(!(FGet(Vars::Aimbot::Hitscan::Modifiers) & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire));
				{
					FSlider("tapfire distance", Vars::Aimbot::Hitscan::TapFireDist, 250.f, 1000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
				}
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Hitscan", true))
				{
					FSlider("bone size subtract", Vars::Aimbot::Hitscan::BoneSizeSubtract, 0.f, 4.f, 0.25f, "%g", FSlider_Min);
					FSlider("bone size minimum scale", Vars::Aimbot::Hitscan::BoneSizeMinimumScale, 0.f, 1.f, 0.1f, "%g", FSlider_Clamp);
				} EndSection();
			}
			if (Section("projectile"))
			{
				FDropdown("predict", Vars::Aimbot::Projectile::StrafePrediction, { "air strafing", "ground strafing" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("splashbot type", Vars::Aimbot::Projectile::SplashPrediction, { "off", "include", "prefer", "only" }, {}, FDropdown_Right);
				FDropdown("sphere calculation method", Vars::Aimbot::Projectile::ComputeType, { "normal", "optimized", "experimental", "hypersphere"}, {}, FDropdown_Left);
				FDropdown("hitboxes## projectile", Vars::Aimbot::Projectile::Hitboxes, { "auto", "head", "body", "feet", "aim projectile at feet" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "projectile hitboxes");
				FDropdown("modifiers## projectile", Vars::Aimbot::Projectile::Modifiers, { "charge shot", "cancel charge", "use prime time" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("projectile splash method##", Vars::Aimbot::Projectile::RocketSplashMode, { "regular", "special light", "special heavy" }, {}, FDropdown_Right);
				FDropdown("delta mode", Vars::Aimbot::Projectile::DeltaMode, { "average", "max" }, {}, FDropdown_Left);
				FDropdown("movesim friction flags", Vars::Aimbot::Projectile::MovesimFrictionFlags, { "run reduce", "calc increase" }, {}, FDropdown_Right | FDropdown_Multi);

				FSlider("max sim time", Vars::Aimbot::Projectile::PredictionTime, 0.1f, 2.5f, 0.25f, "%gs", FSlider_None | FSlider_Min | FSlider_Precision);
				FSlider("splash rx", Vars::Aimbot::Projectile::SplashRotateX, -1.f, 360.f, 1.f, Vars::Aimbot::Projectile::SplashRotateX[DEFAULT_BIND] < 0.f ? "random" : "%g",  FSlider_Left | FSlider_Min);
				FSlider("splash ry", Vars::Aimbot::Projectile::SplashRotateY, -1.f, 360.f, 1.f, Vars::Aimbot::Projectile::SplashRotateY[DEFAULT_BIND] < 0.f ? "random" : "%g", FSlider_Right | FSlider_Min);
				FSlider("splash points", Vars::Aimbot::Projectile::SplashPoints, 1, 400, 5, "%i", FSlider_Left | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::StrafePrediction));
				{
					FSlider("hit chance", Vars::Aimbot::Projectile::Hitchance, 0.f, 100.f, 10.f, "%g%%", FSlider_Right | FSlider_Precision);
				}
				PopTransparent();

				//FSlider("autodet radius", Vars::Aimbot::Projectile::AutodetRadius, 0.f, 100.f, 10.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);

				FSlider("splash radius", Vars::Aimbot::Projectile::SplashRadius, 0.f, 100.f, 10.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::AutoRelease));
				{
					FSlider("auto release", Vars::Aimbot::Projectile::AutoRelease, 0.f, 100.f, 5.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
			} EndSection();

			EndTable();
		}
		break;

		break;

	}
}

void CMenu::MenuHvH(int iTab)
{
	using namespace ImGui;

	// Align widgets to the right side with soft margin
	auto AlignRight = [](float itemWidth = 0.f, float rightMargin = 20.f)
		{
			float columnWidth = GetColumnWidth();
			float offset = columnWidth - itemWidth - GetStyle().ItemSpacing.x - rightMargin;
			if (offset > 0)
				SetCursorPosX(offset);
		};

	switch (iTab)
	{
	case 0:
		if (BeginTable("HvHTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("tickbase manipulation", true))
			{
				FToggle("doubletap", Vars::CL_Move::Doubletap::Doubletap, FToggle_Left);
				FToggle("warp", Vars::CL_Move::Doubletap::Warp, FToggle_Right);
				FToggle("recharge ticks", Vars::CL_Move::Doubletap::RechargeTicks, FToggle_Left);
				FToggle("anti-warp", Vars::CL_Move::Doubletap::AntiWarp, FToggle_Right);
				FToggle("no dt in air", Vars::CL_Move::Doubletap::NoDTinair, FToggle_Left);
				FToggle("only dt at lethal range", Vars::CL_Move::Doubletap::OnlyDTatlethal, FToggle_Right);
				FSlider("tick limit", Vars::CL_Move::Doubletap::TickLimit, 2, 22, 1, "%i", FSlider_Left | FSlider_Clamp);
				FSlider("warp rate", Vars::CL_Move::Doubletap::WarpRate, 2, 22, 1, "%i", FSlider_Right | FSlider_Clamp);
				FSlider("passively recharge", Vars::CL_Move::Doubletap::PassiveRecharge, 0, 67, 1, "%i", FSlider_None | FSlider_Clamp);
			} EndSection();

			if (Section("fakelag"))
			{
				FDropdown("fakelag", Vars::CL_Move::Fakelag::Fakelag, { "off", "plain", "random", "adaptive", "break lc" }, {}, FSlider_Left);
				FDropdown("options", Vars::CL_Move::Fakelag::Options, { "only moving", "on unduck", "not airborne" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "fakelag options");
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Plain);
				{
					FSlider("plain ticks", Vars::CL_Move::Fakelag::PlainTicks, 1, 22, 1, "%i", FSlider_Clamp | FSlider_Left);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Random);
				{
					FSlider("random ticks", Vars::CL_Move::Fakelag::RandomTicks, 1, 22, 1, "%i - %i", FSlider_Clamp | FSlider_Right);
				}
				PopTransparent();
				FToggle("unchoke on attack", Vars::CL_Move::Fakelag::UnchokeOnAttack, FToggle_Left);
				FToggle("retain blastjump", Vars::CL_Move::Fakelag::RetainBlastJump, FToggle_Right);
				FToggle("retain blastjump (soldier only)", Vars::CL_Move::Fakelag::RetainSoldierOnly);
			} EndSection();

			if (Section("resolver", true))
			{
				FToggle("enabled", Vars::AntiHack::Resolver::Enabled, FToggle_Left, nullptr, "resolver enabled");
				PushTransparent(!FGet(Vars::AntiHack::Resolver::Enabled));
				{
					FToggle("auto resolve", Vars::AntiHack::Resolver::AutoResolve, FToggle_Right);
					PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolve));
					{
						FToggle("target cheaters", Vars::AntiHack::Resolver::AutoResolveCheatersOnly, FToggle_Left);
						FToggle("target head", Vars::AntiHack::Resolver::AutoResolveHeadshotOnly, FToggle_Right);
						PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolveYawAmount));
						{
							FSlider("auto resolve yaw", Vars::AntiHack::Resolver::AutoResolveYawAmount, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						}
						PopTransparent();
						PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolvePitchAmount));
						{
							FSlider("auto resolve pitch", Vars::AntiHack::Resolver::AutoResolvePitchAmount, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
						}
						PopTransparent();
					}
					PopTransparent();
					FSlider("cycle yaw angles", Vars::AntiHack::Resolver::CycleYaw, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("cycle pitch", Vars::AntiHack::Resolver::CyclePitch, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
					FToggle("cycle view", Vars::AntiHack::Resolver::CycleView, FToggle_Left);
					FToggle("cycle minwalk", Vars::AntiHack::Resolver::CycleMinwalk, FToggle_Right);
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("anti aim", true))
			{
				FToggle("enabled", Vars::AntiHack::AntiAim::Enabled, FToggle_None, nullptr, "anti-aim");
//				FToggle("auto enable on threat", Vars::AntiHack::AntiAim::AutoEnableOnThreat, FToggle_None, nullptr, "anti-aim"); 
				FDropdown("real pitch", Vars::AntiHack::AntiAim::PitchReal, { "none", "up", "down", "zero", "jitter", "reverse jitter" }, {}, FDropdown_Left);
				FDropdown("fake pitch", Vars::AntiHack::AntiAim::PitchFake, { "none", "up", "down", "jitter", "reverse jitter" }, {}, FDropdown_Right);
				FDropdown("real yaw", Vars::AntiHack::AntiAim::YawReal, { "forward", "left", "right", "backwards", "face edge", "jitter", "spin" }, {}, FDropdown_Left);
				FDropdown("fake yaw", Vars::AntiHack::AntiAim::YawFake, { "forward", "left", "right", "backwards", "face edge", "jitter", "spin" }, {}, FDropdown_Right);
				FDropdown("real offset", Vars::AntiHack::AntiAim::RealYawMode, { "static", "target" }, {}, FDropdown_Left);
				FDropdown("fake offset", Vars::AntiHack::AntiAim::FakeYawMode, { "static", "target" }, {}, FDropdown_Right);
				FSlider("real offset## Offset", Vars::AntiHack::AntiAim::RealYawOffset, -180, 180, 5, "%i", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("fake offset## Offset", Vars::AntiHack::AntiAim::FakeYawOffset, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
				{
					FSlider("real value", Vars::AntiHack::AntiAim::RealYawValue, -180, 180.f, 5.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
				{
					FSlider("fake value", Vars::AntiHack::AntiAim::FakeYawValue, -180.f, 180.f, 5.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Spin && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Spin);
				{
					FSlider("spin speed", Vars::AntiHack::AntiAim::SpinSpeed, -30.f, 30.f, 1.f, "%g", FSlider_Left | FSlider_Precision);
				}
				PopTransparent();
				SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() + H::Draw.Scale(8) });
				FToggle("minwalk", Vars::AntiHack::AntiAim::MinWalk, FToggle_Right); // why was this FToggle_Left? ??
				FToggle("anti-overlap", Vars::AntiHack::AntiAim::AntiOverlap, FToggle_Left);
				FToggle("on shot anti aim", Vars::AntiHack::AntiAim::InvalidShootPitch, FToggle_Right);
			} EndSection();

			if (Section("auto peek", true))
			{
				FToggle("enabled", Vars::CL_Move::AutoPeek, FToggle_Left, nullptr, "autopeek enabled");
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
				FColorPicker("auto peek color", Vars::CL_Move::AutoPeekColor, 0, FColorPicker_Left, nullptr, "autopeek color");
			} EndSection();

			if (Section("fake duck", true))
			{
				FToggle("enabled", Vars::CL_Move::FakeDuck, FToggle_Left, nullptr, "fakeduck enabled");
				FSlider("ticks", Vars::CL_Move::FakeDuckTicks, 1, 12, 1, " %i", FSlider_Clamp); // TBH IDK WHAT TO DO HERE BUT DOING ABOVE 20 IS BAD | grizz anything above 11 is bad if it's 20 you don't fake duck - narrow
			} EndSection();
			EndTable();
		}
		break;
	}
}

void CMenu::MenuAutomations(int iTab)
{
	using namespace ImGui;

	// Align widgets to the right side with soft margin
	auto AlignRight = [](float itemWidth = 0.f, float rightMargin = 20.f)
		{
			float columnWidth = GetColumnWidth();
			float offset = columnWidth - itemWidth - GetStyle().ItemSpacing.x - rightMargin;
			if (offset > 0)
				SetCursorPosX(offset);
		};

	switch (iTab)
	{
	case 0:
		if (BeginTable("automations##", 2, ImGuiTableFlags_SizingStretchSame))
		{
			TableNextColumn();

			if (Section("movement automations##"))
			{
				FToggle("taunt slide", Vars::Misc::Automation::TauntControl, FToggle_Left);
				FToggle("auto jumpbug", Vars::Misc::Movement::AutoJumpbug, FToggle_Right);
				FToggle("auto blastjump", Vars::Misc::Movement::AutoRocketJump, FToggle_Left);
				//FTooltip("if you want to use auto detonator jump you have to crouch");
			}
			EndSection();

			if (Section("auto detonate##"))
			{
				FDropdown("auto detonate", Vars::Aimbot::Projectile::AutoDetonate,
					{ "stickies", "flares", "prevent self harm" /*LOL MELODY NEEDS THIS*/, "ignore cloak" },
					{}, FDropdown_Left | FDropdown_Multi);

				FSlider("detonate radius", Vars::Aimbot::Projectile::AutodetRadius,
					0.f, 100.f, 10.f, "%g%%", FToggle_Right | FSlider_Clamp | FSlider_Precision);
			}
			EndSection();

			if (Section("auto airblast##"))
			{
				FDropdown("auto airblast", Vars::Aimbot::Projectile::AutoAirblast,
					{ "enabled", "redirect simple", "redirect advanced", "respect FOV", "ignore dragons fury" },
					{}, FDropdown_Multi);
			}
			EndSection();

			TableNextColumn();

			if (Section("healing##"))
			{
				FToggle("auto heal", Vars::Aimbot::Healing::AutoHeal, FToggle_Left);

				FToggle("friends only", Vars::Aimbot::Healing::FriendsOnly, FToggle_Right, nullptr, "heal friends only");

				FToggle("activate on voice", Vars::Aimbot::Healing::ActivateOnVoice, FToggle_Left);

				FToggle("auto vaccinator", Vars::Misc::Automation::AutoVaccinator_Active, FToggle_Right);

			}
			EndSection();

			if (Section("misc automations##"))
			{
				FToggle("anti afk", Vars::Misc::Automation::AntiAFK, FToggle_Left);

				FToggle("backpack expander", Vars::Misc::Automation::BackpackExpander, FToggle_Right);

				FToggle("anti autobalance", Vars::Misc::Automation::AntiAutobalance, FToggle_Left);

				FDropdown("anti-backstab", Vars::Misc::Automation::AntiBackstab,
					{ "off", "yaw", "pitch", "fake" }, {});
			}
			EndSection();

			EndTable();
		}
		break;
	}
}

void CMenu::MenuVisuals(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
		// ESP
	case 0:
		if (BeginTable("VisualsESPTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("esp"))
			{
				FDropdown("draw", Vars::ESP::Draw, { "players", "buildings", "projectiles", "objective", "npcs", "health", "ammo", "money", "powerups", "bombs", "spellbook", "gargoyle" }, {}, FDropdown_Multi, 0, nullptr, "draw esp");
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Players));
				{
					FDropdown("player", Vars::ESP::Player, { "enemy", "team", "local", "prioritized", "friends", "party", "name", "box", "distance", "bones", "health bar", "health text", "uber bar", "uber text", "class icon", "class text", "weapon icon", "weapon text", "priority", "labels", "buffs", "debuffs", "misc", "lag compensation", "ping", "kdr" }, {}, FDropdown_Multi, 0, nullptr, "player esp");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Buildings));
				{
					FDropdown("building", Vars::ESP::Building, { "enemy", "team", "local", "prioritized", "friends", "party", "name", "box", "distance", "health bar", "health text", "owner", "level", "flags" }, {}, FDropdown_Multi, 0, nullptr, "building esp");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Projectiles));
				{
					FDropdown("projectile", Vars::ESP::Projectile, { "enemy", "team", "local", "prioritized", "friends", "party",   "name", "box", "distance", "owner", "flags" }, {}, FDropdown_Multi, 0, nullptr, "projectile esp");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Objective));
				{
					FDropdown("objective", Vars::ESP::Objective, { "enemy", "team",   "name", "box", "distance", "flags", "intel return time" }, {}, FDropdown_Multi, 0, nullptr, "objective esp");
				}
				PopTransparent();
			} EndSection();


			/* Column 2 */
			TableNextColumn();
			if (Section("colors", true))
			{
				FToggle("relative colors", Vars::Colors::Relative);
				if (FGet(Vars::Colors::Relative))
				{
					FColorPicker("enemy color", Vars::Colors::Enemy, 0, FColorPicker_Left);
					FColorPicker("team color", Vars::Colors::Team, 0, FColorPicker_Middle);
				}
				else
				{
					FColorPicker("red color", Vars::Colors::TeamRed, 0, FColorPicker_Left);
					FColorPicker("blu color", Vars::Colors::TeamBlu, 0, FColorPicker_Middle);
				}

				FColorPicker("local color", Vars::Colors::Local, 0, FColorPicker_Left);
				FColorPicker("target color", Vars::Colors::Target, 0, FColorPicker_Middle);
				FColorPicker("healthpack color", Vars::Colors::Health, 0, FColorPicker_Left);
				FColorPicker("ammopack color", Vars::Colors::Ammo, 0, FColorPicker_Middle);
				FColorPicker("money color", Vars::Colors::Money, 0, FColorPicker_Left);
				FColorPicker("powerup color", Vars::Colors::Powerup, 0, FColorPicker_Middle);
				FColorPicker("npc color", Vars::Colors::NPC, 0, FColorPicker_Left);
				FColorPicker("halloween color", Vars::Colors::Halloween, 0, FColorPicker_Middle);
				FColorPicker("condition color", Vars::Menu::Theme::Conditions, 0, FColorPicker_Left);
				FColorPicker("health color", Vars::ESP::HPColor, 0, FColorPicker_Middle);

				FToggle("dormant priority only", Vars::ESP::DormantPriority, FToggle_Left);
				FToggle("custom health color", Vars::ESP::CustomHPColor, FToggle_Right);

				FSlider("active alpha", Vars::ESP::ActiveAlpha, 0, 255, 5, "%i", FSlider_Left | FSlider_Clamp);
				FSlider("dormant alpha", Vars::ESP::DormantAlpha, 0, 255, 5, "%i", FSlider_Right | FSlider_Clamp);
				FSlider("dormant decay", Vars::ESP::DormantTime, 0.015f, 5.0f, 0.1f, "%gs", FSlider_Clamp | FSlider_Precision);
			} EndSection();

			EndTable();
		}
		break;
		// Chams
	case 1:
		if (BeginTable("VisualsChamsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();

			{
				if (Section("friendly", true))
				{
					FToggle("players", Vars::Chams::Friendly::Players, FToggle_Left, nullptr, "friendly player chams");
					FToggle("ragdolls", Vars::Chams::Friendly::Ragdolls, FToggle_Right, nullptr, "friendly ragdoll chams");
					FToggle("buildings", Vars::Chams::Friendly::Buildings, FToggle_Left, nullptr, "friendly building chams");
					FToggle("projectiles", Vars::Chams::Friendly::Projectiles, FToggle_Right, nullptr, "friendly projectile chams");

					FMDropdown("visible material", Vars::Chams::Friendly::Visible, FDropdown_None, 0, nullptr, "friendly visible material");
					FMDropdown("occluded material", Vars::Chams::Friendly::Occluded, FDropdown_None, 0, nullptr, "friendly occluded material");
				} EndSection();
				if (Section("enemy", true))
				{
					FToggle("players", Vars::Chams::Enemy::Players, FToggle_Left, nullptr, "enemy player chams");
					FToggle("ragdolls", Vars::Chams::Enemy::Ragdolls, FToggle_Right, nullptr, "enemy ragdoll chams");
					FToggle("buildings", Vars::Chams::Enemy::Buildings, FToggle_Left, nullptr, "enemy building chams");
					FToggle("projectiles", Vars::Chams::Enemy::Projectiles, FToggle_Right, nullptr, "enemy projectile chams");

					FMDropdown("visible material", Vars::Chams::Enemy::Visible, FDropdown_None, 0, nullptr, "enemy visible material");
					FMDropdown("occluded material", Vars::Chams::Enemy::Occluded, FDropdown_None, 0, nullptr, "enemy occluded material");
				} EndSection();
				if (Section("world", true))
				{
					FToggle("npcs", Vars::Chams::World::NPCs, FToggle_Left, nullptr, "npc chams");
					FToggle("pickups", Vars::Chams::World::Pickups, FToggle_Right, nullptr, "pickup chams");
					FToggle("objective", Vars::Chams::World::Objective, FToggle_Left, nullptr, "objective chams");
					FToggle("powerups", Vars::Chams::World::Powerups, FToggle_Right, nullptr, "powerup chams");
					FToggle("bombs", Vars::Chams::World::Bombs, FToggle_Left, nullptr, "bomb chams");
					FToggle("halloween", Vars::Chams::World::Halloween, FToggle_Right, nullptr, "halloween chams");

					FMDropdown("visible material", Vars::Chams::World::Visible, FDropdown_None, 0, nullptr, "world visible material");
					FMDropdown("occluded material", Vars::Chams::World::Occluded, FDropdown_None, 0, nullptr, "world occluded material");
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("player", true))
				{
					FToggle("local", Vars::Chams::Player::Local, FToggle_Left, nullptr, "local chams");
					FToggle("priority", Vars::Chams::Player::Priority, FToggle_Right, nullptr, "priority chams");
					FToggle("friend", Vars::Chams::Player::Friend, FToggle_Left, nullptr, "friend chams");
					FToggle("party", Vars::Chams::Player::Party, FToggle_Right, nullptr, "party chams");
					FToggle("target", Vars::Chams::Player::Target, FToggle_Left, nullptr, "target chams");

					FMDropdown("visible material", Vars::Chams::Player::Visible, FDropdown_None, 0, nullptr, "player visible material");
					FMDropdown("occluded material", Vars::Chams::Player::Occluded, FDropdown_None, 0, nullptr, "player occluded material");
				} EndSection();
				if (Section("backtrack", true))
				{
					FToggle("enabled", Vars::Chams::Backtrack::Enabled, FToggle_Left, nullptr, "backtrack chams");
					FToggle("ignore z", Vars::Chams::Backtrack::IgnoreZ, FToggle_Right, nullptr, "backtrack ignore z");

					FMDropdown("material", Vars::Chams::Backtrack::Visible, FDropdown_None, 0, nullptr, "backtrack material");
					FDropdown("draw", Vars::Chams::Backtrack::Draw, { "last", "last + first", "all" }, {}, FDropdown_None, 0, nullptr, "backtrack chams mode");
				} EndSection();
				if (Section("fake angle", true))
				{
					FToggle("enabled", Vars::Chams::FakeAngle::Enabled, FToggle_Left, nullptr, "fake angle chams");
					FToggle("ignore z", Vars::Chams::FakeAngle::IgnoreZ, FToggle_Right, nullptr, "fake angle ignore z");

					FMDropdown("material", Vars::Chams::FakeAngle::Visible, FDropdown_None, 0, nullptr, "fake angle material");
				} EndSection();
				if (Section("viewmodel", true))
				{
					FToggle("weapon", Vars::Chams::Viewmodel::Weapon, FToggle_Left, nullptr, "weapon chams");
					FToggle("hands", Vars::Chams::Viewmodel::Hands, FToggle_Right, nullptr, "hands chams");

					FMDropdown("weapon material", Vars::Chams::Viewmodel::WeaponVisible, FDropdown_None, 0, nullptr, "weapon material");
					FMDropdown("hands material", Vars::Chams::Viewmodel::HandsVisible, FDropdown_None, 0, nullptr, "hands material");
				} EndSection();

				EndTable();
			}
			break;
			// Glow
	case 2:
		if (BeginTable("VisualsGlowTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("friendly", true))
			{
				FToggle("players", Vars::Glow::Friendly::Players, FToggle_Left, nullptr, "friendly player glow");
				FToggle("ragdolls", Vars::Glow::Friendly::Ragdolls, FToggle_Right, nullptr, "friendly ragdoll glow");
				FToggle("buildings", Vars::Glow::Friendly::Buildings, FToggle_Left, nullptr, "friendly building glow");
				FToggle("projectiles", Vars::Glow::Friendly::Projectiles, FToggle_Right, nullptr, "friendly projectile glow");

				PushTransparent(!FGet(Vars::Glow::Friendly::Stencil));
				{
					FSlider("stencil scale## friendly", Vars::Glow::Friendly::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "friendly stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Friendly::Blur));
				{
					FSlider("blur scale## friendly", Vars::Glow::Friendly::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "friendly blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("enemy", true))
			{
				FToggle("players", Vars::Glow::Enemy::Players, FToggle_Left, nullptr, "enemy player glow");
				FToggle("ragdolls", Vars::Glow::Enemy::Ragdolls, FToggle_Right, nullptr, "enemy ragdoll glow");
				FToggle("buildings", Vars::Glow::Enemy::Buildings, FToggle_Left, nullptr, "enemy building glow");
				FToggle("projectiles", Vars::Glow::Enemy::Projectiles, FToggle_Right, nullptr, "enemy projectile glow");

				PushTransparent(!FGet(Vars::Glow::Enemy::Stencil));
				{
					FSlider("stencil scale## enemy", Vars::Glow::Enemy::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "enemy stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Enemy::Blur));
				{
					FSlider("blur scale## enemy", Vars::Glow::Enemy::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "enemy blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("world", true))
				FToggle("NPC's", Vars::Glow::World::NPCs, FToggle_Left, nullptr, "NPC glow");
			FToggle("pickups", Vars::Glow::World::Pickups, FToggle_Right, nullptr, "pickup glow");
			FToggle("objective", Vars::Glow::World::Objective, FToggle_Left, nullptr, "objective glow");
			FToggle("powerups", Vars::Glow::World::Powerups, FToggle_Right, nullptr, "powerup glow");
			FToggle("bombs", Vars::Glow::World::Bombs, FToggle_Left, nullptr, "bomb glow");
			FToggle("halloween", Vars::Glow::World::Halloween, FToggle_Right, nullptr, "halloween glow");

			PushTransparent(!FGet(Vars::Glow::World::Stencil));
			{
				FSlider("stencil scale## world", Vars::Glow::World::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "world stencil scale");
			}
			PopTransparent();
			PushTransparent(!FGet(Vars::Glow::World::Blur));
			{
				FSlider("blur scale## world", Vars::Glow::World::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "world blur scale");
			}
			PopTransparent();
		} EndSection();

		/* Column 2 */
		TableNextColumn();
		if (Section("player", true))
		{
			FToggle("local", Vars::Glow::Player::Local, FToggle_Left, nullptr, "local glow");
			FToggle("priority", Vars::Glow::Player::Priority, FToggle_Right, nullptr, "priority glow");
			FToggle("friend", Vars::Glow::Player::Friend, FToggle_Left, nullptr, "friend glow");
			FToggle("party", Vars::Glow::Player::Party, FToggle_Right, nullptr, "party glow");
			FToggle("target", Vars::Glow::Player::Target, FToggle_Left, nullptr, "target glow");

			PushTransparent(!FGet(Vars::Glow::Player::Stencil));
			{
				FSlider("stencil scale## player", Vars::Glow::Player::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "player stencil scale");
			}
			PopTransparent();
			PushTransparent(!FGet(Vars::Glow::Player::Blur));
			{
				FSlider("blur scale## player", Vars::Glow::Player::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "player blur scale");
			}
			PopTransparent();
		} EndSection();
		if (Section("backtrack", true))
		{
			FToggle("enabled", Vars::Glow::Backtrack::Enabled, FToggle_Left, nullptr, "backtrack glow");
			PushTransparent(!FGet(Vars::Colors::Backtrack).a);
			{
				FColorPicker("color", Vars::Colors::Backtrack, 0, FColorPicker_Middle, nullptr, "backtrack color");
			}
			PopTransparent();

			PushTransparent(!FGet(Vars::Glow::Backtrack::Stencil));
			{
				FSlider("stencil scale## backtrack", Vars::Glow::Backtrack::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "backtrack stencil scale");
			}
			PopTransparent();
			PushTransparent(!FGet(Vars::Glow::Backtrack::Blur));
			{
				FSlider("blur scale## backtrack", Vars::Glow::Backtrack::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "backtrack blur scale");
			}
			PopTransparent();
			FDropdown("draw", Vars::Glow::Backtrack::Draw, { "last", "last + first", "all" }, {}, FDropdown_None, 0, nullptr, "backtrack glow mode");
		} EndSection();
		if (Section("fake angle", true))
		{
			FToggle("enabled", Vars::Glow::FakeAngle::Enabled, FToggle_Left, nullptr, "fake angle glow");
			PushTransparent(!FGet(Vars::Colors::FakeAngle).a);
			{
				FColorPicker("color", Vars::Colors::FakeAngle, 0, FColorPicker_Middle, nullptr, "Fake angle color");
			}
			PopTransparent();

			PushTransparent(!FGet(Vars::Glow::FakeAngle::Stencil));
			{
				FSlider("stencil scale## fakeAngle", Vars::Glow::FakeAngle::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "fake angle stencil scale");
			}
			PopTransparent();
			PushTransparent(!FGet(Vars::Glow::FakeAngle::Blur));
			{
				FSlider("blur scale## fakeangle", Vars::Glow::FakeAngle::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "fake angle blur scale");
			}
			PopTransparent();
		} EndSection();
		if (Section("viewmodel", true))
		{
			FToggle("weapon", Vars::Glow::Viewmodel::Weapon, FToggle_Left, nullptr, "Weapon glow");
			FToggle("hands", Vars::Glow::Viewmodel::Hands, FToggle_Right, nullptr, "Hands glow");

			PushTransparent(!FGet(Vars::Glow::Viewmodel::Stencil));
			{
				FSlider("stencil scale## viewmodel", Vars::Glow::Viewmodel::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Viewmodel stencil scale");
			}
			PopTransparent();
			PushTransparent(!FGet(Vars::Glow::Viewmodel::Blur));
			{
				FSlider("blur scale## viewmodel", Vars::Glow::Viewmodel::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "viewmodel blur scale");
			}
			PopTransparent();
		} EndSection();

		EndTable();
		}
		break;
		// Misc
	case 3:
		if (BeginTable("VisualsMiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("removals", true))
			{
				FToggle("scope", Vars::Visuals::Removals::Scope, FToggle_Left, nullptr, "Scope removal");
				FToggle("interpolation", Vars::Visuals::Removals::Interpolation, FToggle_Right, nullptr, "Interpolation removal");
				FToggle("disguises", Vars::Visuals::Removals::Disguises, FToggle_Left, nullptr, "Disguises removal");
				FToggle("screen overlays", Vars::Visuals::Removals::ScreenOverlays, FToggle_Right, nullptr, "Screen overlays removal");
				FToggle("taunts", Vars::Visuals::Removals::Taunts, FToggle_Left, nullptr, "Taunts removal");
				FToggle("screen effects", Vars::Visuals::Removals::ScreenEffects, FToggle_Right, nullptr, "Screen effects removal");
				FToggle("view punch", Vars::Visuals::Removals::ViewPunch, FToggle_Left, nullptr, "View punch removal");
				FToggle("angle forcing", Vars::Visuals::Removals::AngleForcing, FToggle_Right, nullptr, "Angle forcing removal");
				FToggle("post processing", Vars::Visuals::Removals::PostProcessing, FToggle_Left, nullptr, "Post processing removal");
				FToggle("MOTD", Vars::Visuals::Removals::MOTD, FToggle_Right, nullptr, "MOTD removal");
			} EndSection();
			if (Section("UI"))
			{
				FDropdown("hide names", Vars::Visuals::UI::StreamerMode, { "off", "local", "friends", "party", "all" }, {}, FDropdown_Left);
				FDropdown("chat tags", Vars::Visuals::UI::ChatTags, { "local", "friends", "party", "assigned" }, {}, FDropdown_Right | FDropdown_Multi);
				PushTransparent(!FGet(Vars::Visuals::UI::FieldOfView));
				{
					FSlider("field of view", Vars::Visuals::UI::FieldOfView, 0.f, 160.f, 1.f, "%g", FSlider_Min);
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::UI::ZoomFieldOfView));
				{
					FSlider("zoomed field of view", Vars::Visuals::UI::ZoomFieldOfView, 0.f, 160.f, 1.f, "%g", FSlider_Min);
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::UI::AspectRatio));
				{
					FSlider("aspect ratio", Vars::Visuals::UI::AspectRatio, 0.f, 5.f, 0.01f, "%g", FSlider_Min | FSlider_Precision);
				}
				PopTransparent();
				FToggle("reveal scoreboard", Vars::Visuals::UI::RevealScoreboard, FToggle_Left);
				FToggle("scoreboard utility", Vars::Visuals::UI::ScoreboardUtility, FToggle_Right);
				FToggle("scoreboard colors", Vars::Visuals::UI::ScoreboardColors, FToggle_Left);
				FToggle("clean screenshots", Vars::Visuals::UI::CleanScreenshots, FToggle_Right);
				FToggle("sniper sightlines", Vars::Visuals::UI::SniperSightlines, FToggle_Left);
				FToggle("pickup timers", Vars::Visuals::UI::PickupTimers, FToggle_Right);
				FToggle("netgraph watermark", Vars::Visuals::UI::NetGraphWatermark, FToggle_Left);
			} EndSection();


			if (Section("freecam"))
			{
				FToggle("freecam", Vars::Misc::Game::Freecam, FToggle_Left);
				FSlider("freecam speed", Vars::Misc::Game::FreecamSpeed, 0, 100);
			} EndSection();


			if (Section("hitbox"))
			{
				FDropdown("bones enabled", Vars::Visuals::Hitbox::BonesEnabled, { "on shot", "on hit" }, {}, FDropdown_Multi | FDropdown_Left, 0, nullptr, "hitbox bones enabled");

				FDropdown("bounds enabled", Vars::Visuals::Hitbox::BoundsEnabled, { "on shot", "on hit", "aim point"}, {}, FDropdown_Multi | FDropdown_Right, 0, nullptr, "hitbox bones enabled");

				FSlider("draw duration## hitbox", Vars::Visuals::Hitbox::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "hitbox draw duration");
			} EndSection();
			if (Section("thirdperson", true))
			{
				FToggle("thirdperson", Vars::Visuals::ThirdPerson::Enabled, FToggle_Left);
				FToggle("thirdperson crosshair", Vars::Visuals::ThirdPerson::Crosshair, FToggle_Right);
				FToggle("thirdperson collision", Vars::Visuals::ThirdPerson::Collide, FToggle_Left);
				FSlider("thirdperson up", Vars::Visuals::ThirdPerson::Up, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("thirdperson right", Vars::Visuals::ThirdPerson::Right, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("thirdperson distance", Vars::Visuals::ThirdPerson::Distance, 0.f, 500.f, 5.f, "%g", FSlider_Precision);
			} EndSection();
			if (Section("world"))
			{
				//FSDropdown("world texture", Vars::Visuals::World::WorldTexture, { "default", "dev", "camo", "black", "white", "flat" }, FSDropdown_Custom);
				FDropdown("modulations", Vars::Visuals::World::Modulations, { "world", "sky", "prop", "particle", "fog" }, {}, FDropdown_Left | FDropdown_Multi);
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::World));
				{
					FColorPicker("world modulation", Vars::Colors::WorldModulation, 0, FColorPicker_Left);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Sky));
				{
					FColorPicker("sky modulation", Vars::Colors::SkyModulation, 0, FColorPicker_Middle);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Prop));
				{
					FColorPicker("prop modulation", Vars::Colors::PropModulation, 0, FColorPicker_Left);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Particle));
				{
					FColorPicker("particle modulation", Vars::Colors::ParticleModulation, 0, FColorPicker_Middle);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Fog));
				{
					FColorPicker("fog modulation", Vars::Colors::FogModulation, 0, FColorPicker_Left);
				}
				PopTransparent();
				FToggle("near prop fade", Vars::Visuals::World::NearPropFade, FToggle_Left);
				FToggle("no prop fade", Vars::Visuals::World::NoPropFade, FToggle_Right);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("line", true))
			{
				FColorPicker("line tracer clipped", Vars::Colors::LineClipped, 0, FColorPicker_None, nullptr, "Line tracer clipped color");
				FColorPicker("line tracer", Vars::Colors::Line, 1, FColorPicker_None, nullptr, "Line tracer color");
				FToggle("line tracers", Vars::Visuals::Line::Enabled);
				FSlider("draw duration## Line", Vars::Visuals::Line::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "Line draw duration");
			} EndSection();
			if (Section("simulation"))
			{
				FDropdown("player path", Vars::Visuals::Simulation::PlayerPath, { "off", "line", "separators", "spaced", "arrows", "boxes", "outlined" }, {}, FDropdown_Left, -20);
				FColorPicker("player path", Vars::Colors::PlayerPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "player path color");
				FColorPicker("player path clipped", Vars::Colors::PlayerPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Player path clipped color");
				FDropdown("projectile path", Vars::Visuals::Simulation::ProjectilePath, { "off", "line", "separators", "spaced", "arrows", "boxes" }, {}, FDropdown_Right, -20);
				FColorPicker("projectile path", Vars::Colors::ProjectilePath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Projectile path color");
				FColorPicker("projectile path clipped", Vars::Colors::ProjectilePathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Projectile path clipped color");
				FDropdown("trajectory path", Vars::Visuals::Simulation::TrajectoryPath, { "off", "line", "separators", "spaced", "arrows", "boxes" }, {}, FDropdown_Left, -20);
				FColorPicker("trajectory path", Vars::Colors::TrajectoryPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Trajectory path color");
				FColorPicker("trajectory path clipped", Vars::Colors::TrajectoryPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Trajectory path clipped color");
				FDropdown("shot path", Vars::Visuals::Simulation::ShotPath, { "off", "line", "separators", "spaced", "arrows", "boxes" }, {}, FDropdown_Right, -20);
				FColorPicker("shot path", Vars::Colors::ShotPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "shot path color");
				FColorPicker("shot path clipped", Vars::Colors::ShotPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Shot path clipped color");
				FDropdown("splash radius", Vars::Visuals::Simulation::SplashRadius, { "simulation", "##Divider", "priority", "enemy", "team", "local", "friends", "party", "##Divider",  "rockets", "stickies", "pipes", "scorch shot", "##Divider",  "collisions" }, {}, FDropdown_Multi, -20);
				FColorPicker("splash radius", Vars::Colors::SplashRadius, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Splash radius color");
				FColorPicker("splash radius clipped", Vars::Colors::SplashRadiusClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Splash radius color");
				FToggle("timed", Vars::Visuals::Simulation::Timed, FToggle_Left, nullptr, "timed path");
				FToggle("box", Vars::Visuals::Simulation::Box, FToggle_Right, nullptr, "path box");
				FToggle("projectile camera", Vars::Visuals::Simulation::ProjectileCamera, FToggle_Left);
				FToggle("swing prediction lines", Vars::Visuals::Simulation::SwingLines, FToggle_Right);
				PushTransparent(FGet(Vars::Visuals::Simulation::Timed));
				{
					FSlider("draw duration## simulation", Vars::Visuals::Simulation::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "Simulation draw duration");
				}
				PopTransparent();
			} EndSection();

			/*if (Section("skinchanger", true))
			{
				if (FButton("configure skinchanger (pasted from m-fed)", FButton_None))
				{
					ShellExecuteA(
						nullptr,        // no parent window
						"open",         // operation
						"http://localhost:4077/",
						nullptr,        // no parameters
						nullptr,        // default working directory
						SW_SHOWNORMAL   // normal window
					);
				}
			}
			EndSection();*/

			if (Section("Out Of FOV arrows", true))
			{
				FToggle("enabled", Vars::Visuals::FOVArrows::Enabled, FToggle_None, nullptr, "OOF arrows");
				FSlider("offset", Vars::Visuals::FOVArrows::Offset, 0, 500, 25, "%i", FSlider_Left | FSlider_Min | FSlider_Precision, nullptr, "OOF arrows offset");
				FSlider("max distance", Vars::Visuals::FOVArrows::MaxDist, 0.f, 5000.f, 50.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision, nullptr, "OOF arrows max distance");
				// dumb fucking nigger shit man
				FDropdown("oob style", Vars::Visuals::FOVArrows::OOBStyle, { "diamond", "arrow" }, {}, FDropdown_None, -20);
			} EndSection();

			if (Section("viewmodel", true))
			{
				FToggle("crosshair aim position", Vars::Visuals::Viewmodel::CrosshairAim, FToggle_Left);
				FToggle("viewmodel aim position", Vars::Visuals::Viewmodel::ViewmodelAim, FToggle_Right);
				FSlider("offset X", Vars::Visuals::Viewmodel::OffsetX, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset x");
				FSlider("pitch", Vars::Visuals::Viewmodel::Pitch, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel pitch");
				FSlider("offset Y", Vars::Visuals::Viewmodel::OffsetY, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset y");
				FSlider("yaw", Vars::Visuals::Viewmodel::Yaw, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel yaw");
				FSlider("offset Z", Vars::Visuals::Viewmodel::OffsetZ, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset z");
				FSlider("roll", Vars::Visuals::Viewmodel::Roll, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel roll");
				PushTransparent(!FGet(Vars::Visuals::Viewmodel::FieldOfView));
				{
					FSlider("field of view## viewmodel", Vars::Visuals::Viewmodel::FieldOfView, 0.f, 180.f, 1.f, "%.0f", FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel field of view");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::Viewmodel::SwayScale) || !FGet(Vars::Visuals::Viewmodel::SwayInterp));
				{
					FSlider("sway scale", Vars::Visuals::Viewmodel::SwayScale, 0.f, 5.f, 0.5f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision, nullptr, "Viewmodel sway scale");
					FSlider("sway interp", Vars::Visuals::Viewmodel::SwayInterp, 0.f, 1.f, 0.1f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision, nullptr, "Viewmodel sway interp");
				}
				PopTransparent();
			} EndSection();

			if (Section("killfeed", true))
			{
				FDropdown("icon", Vars::Visuals::Killfeed::Icon, { "none", "sawblade", "pumpkin", "armageddon", "train", "skull", "saxxy"}, {}, FDropdown_None);
				FDropdown("overlays", Vars::Visuals::Killfeed::Overlays, { "none", "australium", "crit" }, {}, FDropdown_None);
				FDropdown("force domination", Vars::Visuals::Killfeed::Domination, { "none", "domination", "revenge", "both" }, {}, FDropdown_None);
			} EndSection();

			if (Section("skinchanger", true))
			{
				FToggle("enable", Vars::Visuals::SkinChanger::Enabled, FToggle_Left);
				FToggle("festivized", Vars::Visuals::SkinChanger::Festivized, FToggle_Right);
				FToggle("australium", Vars::Visuals::SkinChanger::Australium, FToggle_Left);
			//	FDropdown("weapon unusual", Vars::Visuals::SkinChanger::Unusual, { "none", "cool", "hot", "isotope", "energy orb"}, {}, FDropdown_None);
			} EndSection();

			EndTable();
		}
		break;

	case 4:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("logging"))
			{
				FDropdown("logs", Vars::Logging::Logs, { "vote start", "vote cast", "class changes", "damage", "cheat detection", "tags", "aliases", "resolver", "vaccinator" }, {}, FDropdown_Multi);
				FSlider("notification time", Vars::Logging::Lifetime, 0.5f, 5.f, 0.5f, "%g");
			} EndSection();
			if (Section("vote start"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteStart));
				{
					FDropdown("log to", Vars::Logging::VoteStart::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "vote start log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("vote cast"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteCast));
				{
					FDropdown("log to", Vars::Logging::VoteCast::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "vote cast log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("class change"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::ClassChanges));
				{
					FDropdown("log to", Vars::Logging::ClassChange::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "class change log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("auto vaccinator"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Vaccinator));
				{
					FDropdown("log to", Vars::Logging::Vaccinator::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "vaccinator log to");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("damage"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Damage));
				{
					FDropdown("log to", Vars::Logging::Damage::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "damage log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("cheat detection"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::CheatDetection));
				{
					FDropdown("log to", Vars::Logging::CheatDetection::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "cheat detection log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("tags"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Tags));
				{
					FDropdown("log to", Vars::Logging::Tags::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "tags log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("aliases"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Aliases));
				{
					FDropdown("log to", Vars::Logging::Aliases::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "aliases log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("resolver"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Resolver));
				{
					FDropdown("log to", Vars::Logging::Resolver::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "resolver log to");
				}
				PopTransparent();
			} EndSection();

			EndTable();
		}
		break;
		// Menu
	case 5:
		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();

			if (Section("menu", true))
			{
				FColorPicker("accent color", Vars::Menu::Theme::Accent, 0, FColorPicker_Left);
				//			FColorPicker("Active color", Vars::Menu::Theme::Active, 0, FColorPicker_Left);
				//			FColorPicker("Inactive color", Vars::Menu::Theme::Inactive, 0, FColorPicker_Middle);

				FKeybind("primary key", Vars::Menu::MenuPrimaryKey[DEFAULT_BIND], FButton_Left | FKeybind_AllowMenu);
				FKeybind("secondary key", Vars::Menu::MenuSecondaryKey[DEFAULT_BIND], FButton_Right | FButton_SameLine | FKeybind_AllowMenu);
				switch (Vars::Menu::MenuPrimaryKey[DEFAULT_BIND])
				{
				case VK_LBUTTON:
				case VK_RBUTTON:
					Vars::Menu::MenuPrimaryKey[DEFAULT_BIND] = Vars::Menu::MenuPrimaryKey.Default;
				}
				switch (Vars::Menu::MenuSecondaryKey[DEFAULT_BIND])
				{
				case VK_LBUTTON:
				case VK_RBUTTON:
					Vars::Menu::MenuSecondaryKey[DEFAULT_BIND] = Vars::Menu::MenuSecondaryKey.Default;
				}
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("indicators"))
			{
				FToggle("remove background", Vars::Menu::RemoveBackground, FToggle_Left);
				FDropdown("indicators", Vars::Menu::Indicators, { "doubletap", "crithack", "speclist", "ping", "seed prediction", "watermark", "spotify", "movement recorder"}, {}, FDropdown_Multi);      // Ok narov

			} EndSection();
			EndTable();
		}
		break;
	}
}

void CMenu::MenuMisc(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	case 0:
		if (BeginTable("MiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("movement"))
			{
				FDropdown("auto strafe", Vars::Misc::Movement::AutoStrafe, { "off", "assist", "directional" });
				PushTransparent(FGet(Vars::Misc::Movement::AutoStrafe) != Vars::Misc::Movement::AutoStrafeEnum::Directional);
				{
					FSlider("auto strafe turn scale", Vars::Misc::Movement::AutoStrafeTurnScale, 0.f, 1.f, 0.1f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("auto strafe max delta", Vars::Misc::Movement::AutoStrafeMaxDelta, 0.f, 180.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				FToggle("bunnyhop", Vars::Misc::Movement::Bunnyhop, FToggle_Left);
				FToggle("edge jump", Vars::Misc::Movement::EdgeJump, FToggle_Right);
				FToggle("remove push", Vars::Misc::Movement::NoPush, FToggle_Left);
				// FToggle("Auto ctap", Vars::Misc::Movement::AutoCTap, FToggle_Right);
				FToggle("fast stop", Vars::Misc::Movement::FastStop, FToggle_Right);
				// FToggle("Fast accelerate", Vars::Misc::Movement::FastAccel, FToggle_Right);
				// FToggle("Crouch speed", Vars::Misc::Movement::CrouchSpeed, FToggle_Left);
				// FToggle("Movement lock", Vars::Misc::Movement::MovementLock, FToggle_Right);
				FToggle("break jump", Vars::Misc::Movement::BreakJump, FToggle_Left);
				FToggle("shield turn rate", Vars::Misc::Movement::ShieldTurnRate, FToggle_Right);
				//FToggle("movebot", Vars::Misc::Movement::NavBot, FToggle_Left);
				//FToggle("movebot avoid walls", Vars::Misc::Movement::NavBotAvoidWalls, FToggle_Right);
				FToggle("block bot", Vars::Misc::Movement::BlockBot, FToggle_Left);
				FToggle("block bot snipers only", Vars::Misc::Movement::BlockBotSnipersOnly, FToggle_Right);
				FSlider("block bot offset", Vars::Misc::Movement::BlockBotOffset, 5.f, 50.f, 1.f, "%g", FSlider_Clamp | FSlider_Precision);
			} EndSection();

			if (Section("exploits", true))
			{
				FToggle("sv_cheats bypass", Vars::Misc::Exploits::CheatsBypass, FToggle_Left);
				FToggle("pure bypass", Vars::Misc::Exploits::BypassPure, FToggle_Right);
				FToggle("ping reducer", Vars::Misc::Exploits::PingReducer, FToggle_Left);
				FToggle("equip region unlock", Vars::Misc::Exploits::EquipRegionUnlock, FToggle_Right);
				PushTransparent(!FGet(Vars::Misc::Exploits::PingReducer));
				{
					FSlider("ping target", Vars::Misc::Exploits::PingTarget, 1, 66, 1, "%i", FSlider_Clamp);
				}

				if (Section("backtrack", true))
				{
					FToggle("enabled", Vars::Backtrack::Enabled, FToggle_Left, nullptr, "backtrack enabled");
					FToggle("prefer on shot", Vars::Backtrack::PreferOnShot, FToggle_Right);
					FSlider("fake latency", Vars::Backtrack::Latency, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp);
					FSlider("fake interp", Vars::Backtrack::Interp, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp | FSlider_Precision);
					FSlider("window", Vars::Backtrack::Window, 1, 200, 5, "%i", FSlider_Clamp, nullptr, "backtrack window");
				} EndSection();

				PopTransparent();
			} EndSection(); // Added missing EndSection() to close the "exploits" section

			/* Column 2 */
			TableNextColumn();
			if (Section("game", true))
			{
				FToggle("network fix", Vars::Misc::Game::NetworkFix, FToggle_Left);
				FToggle("prediction error jitter fix", Vars::Misc::Game::PredictionErrorJitterFix, FToggle_Right);
				FToggle("bones optimization", Vars::Misc::Game::SetupBonesOptimization, FToggle_Left);
				FToggle("anti cheat compatibility", Vars::Misc::Game::AntiCheatCompatibility);
				FToggle("anti cheat crit hack (untrusted)", Vars::Misc::Game::AntiCheatCritHack);
			} EndSection();

			if (Section("queueing"))
			{
				FToggle("freeze queue", Vars::Misc::Queueing::FreezeQueue, FToggle_Left);
				FDropdown("force regions", Vars::Misc::Queueing::ForceRegions,
					{ "atlanta", "chicago", "texas", "los angeles", "moses lake", "new york", "seattle", "virginia", "amsterdam", "frankfurt", "helsinki", "london", "madrid", "paris", "stockholm", "vienna", "warsaw",   "buenos aires", "lima", "santiago", "sao paulo",   "dombay", "chennai", "dubai", "hong kong", "madras", "mumbai", "seoul", "singapore", "tokyo", "sydney",   "johannesburg" },
					{}, FDropdown_Multi
				);
			} EndSection();

			if (Section("convar spoofer"))
			{
				static std::string sName = "", sValue = "";

				FSDropdown("convar", &sName, {}, FDropdown_Left);
				FSDropdown("value", &sValue, {}, FDropdown_Right);
				if (FButton("send"))
				{
					if (auto pNetChan = static_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo()))
					{
						SDK::Output("convar", std::format("sent {} as {}", sName, sValue).c_str(), Vars::Menu::Theme::Accent.Value);
						NET_SetConVar cmd(sName.c_str(), sValue.c_str());
						pNetChan->SendNetMsg(cmd);

						sName = sValue = "";
					}
				}
			} EndSection();

			if (Section("sound"))
			{
				FToggle("hitsound always", Vars::Misc::Sound::HitsoundAlways, FToggle_Left);
				FToggle("remove DSP", Vars::Misc::Sound::RemoveDSP, FToggle_Right);
				FToggle("giant weapon sounds", Vars::Misc::Sound::GiantWeaponSounds);
				FDropdown("block", Vars::Misc::Sound::Block, { "footsteps", "noisemaker", "frying pan", "water", "door" }, {}, FDropdown_Multi);
			} EndSection();

			EndTable();
		}
		break;
		case 1:
			if (BeginTable("MiscTable1", 2))
			{
				TableNextColumn();
				if (Section("main"))
				{
					FToggle("enabled", Vars::MovementRecorder::Enabled, FToggle_Left);
					FToggle("playback", Vars::MovementRecorder::Playback, FToggle_Right);
					FToggle("save", Vars::MovementRecorder::Save, FToggle_Left);
				} EndSection();

				// autism
				if (Section("visualize"))
				{
					FToggle("line", Vars::MovementRecorder::Line, FToggle_Left);
					ImGui::SameLine(150);
					FToggle("visualize spot", Vars::MovementRecorder::VisualizeSpot, FToggle_Left);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
					FColorPicker("line color", Vars::MovementRecorder::LineColor, 0, FColorPicker_Left, nullptr, "line color");
					ImGui::SameLine(150); 
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
					FColorPicker("spot color", Vars::MovementRecorder::VisualizeSpotColor, 0, FColorPicker_Left, nullptr, "spot color");
				}
				EndSection();

				TableNextColumn();
				if (Section("config"))
				{
					static std::string newName;
					FSDropdown("config name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);

					if (FButton("create", FButton_Fit | FButton_SameLine, { 0, 40 }) && !newName.empty())
					{
						const auto fullPath = F::Configs.m_sConfigPath + newName + ".mvr";
						if (!std::filesystem::exists(fullPath))
						{
							F::Configs.SaveMovementShid(newName);
						}
						newName.clear();
					}

					std::vector<std::pair<std::filesystem::directory_entry, std::string>> vMovementConfigs;
					for (const auto& entry : std::filesystem::directory_iterator(F::Configs.m_sConfigPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != ".mvr")
							continue;

						std::string name = entry.path().stem().string();
						vMovementConfigs.emplace_back(entry, name);
					}

					std::sort(vMovementConfigs.begin(), vMovementConfigs.end(),
						[](const auto& a, const auto& b) { return a.second < b.second; });

					for (auto& [entry, name] : vMovementConfigs)
					{
						bool bCurrent = F::Configs.m_sCurrentConfig == name;

						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(bCurrent ? F::Render.Active.Value : F::Render.Inactive.Value,
							TruncateText(name, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

						int iOffset = 0;
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(1) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
						{
							F::Configs.RemoveConfigShid(name);
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(2) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_SAVE))
						{
							F::Configs.SaveMovementShid(name);
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(3) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DOWNLOAD))
						{
							F::Configs.LoadMovementShid(name);
						}


						SetCursorPos(vOriginalPos);
						DebugDummy({ 0, H::Draw.Scale(28) });
					}
				}
				EndSection();
				EndTable();

			}
			break;
	}
}


void CMenu::MenuLogs(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
		// Settings
	case 0:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("logging"))
			{
				FDropdown("logs", Vars::Logging::Logs, { "vote start", "vote cast", "class changes", "damage", "cheat detection", "tags", "aliases", "resolver" }, {}, FDropdown_Multi);
				FSlider("notification time", Vars::Logging::Lifetime, 0.5f, 5.f, 0.5f, "%g");
			} EndSection();
			if (Section("vote start"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteStart));
				{
					FDropdown("log to", Vars::Logging::VoteStart::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "Vote start log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("vote cast"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteCast));
				{
					FDropdown("log to", Vars::Logging::VoteCast::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "Vote cast log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("class change"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::ClassChanges));
				{
					FDropdown("log to", Vars::Logging::ClassChange::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "Class change log to");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("damage"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Damage));
				{
					FDropdown("log to", Vars::Logging::Damage::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "damage log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("cheat detection"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::CheatDetection));
				{
					FDropdown("log to", Vars::Logging::CheatDetection::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "cheat detection log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("tags"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Tags));
				{
					FDropdown("log to", Vars::Logging::Tags::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "tags log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("aliases"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Aliases));
				{
					FDropdown("log to", Vars::Logging::Aliases::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "aliases log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("resolver"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Resolver));
				{
					FDropdown("Log to", Vars::Logging::Resolver::LogTo, { "toasts", "chat", "party", "console", "menu" }, {}, FDropdown_Multi, 0, nullptr, "resolver log to");
				}
				PopTransparent();
			} EndSection();

			EndTable();
		}
		break;
		// Output
	case 1:
		if (Section("##output", false, GetWindowHeight() - GetStyle().WindowPadding.y * 2))
		{
			for (auto& tOutput : m_vOutput)
			{
				ImVec2 vOriginalPos = GetCursorPos();
				size_t iLines = 1;

				float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2;
				if (tOutput.m_sFunction != "")
				{
					float flTitleWidth = 0.f;

					PushStyleColor(ImGuiCol_Text, ColorToVec(tOutput.tAccent));

					auto vWrapped = WrapText(tOutput.m_sFunction, flWidth);
					for (size_t i = 0; i < vWrapped.size(); i++)
					{
						FText(vWrapped[i].c_str());
						if (i == vWrapped.size() - 1)
							flTitleWidth = FCalcTextSize(vWrapped[i].c_str()).x + H::Draw.Scale(4);
					}
					iLines = vWrapped.size();

					PopStyleColor();

					vWrapped = WrapText(tOutput.m_sLog, flWidth - flTitleWidth);
					if (!vWrapped.empty())
					{
						SameLine(flTitleWidth + GetStyle().WindowPadding.x);
						FText(vWrapped.front().c_str());

						if (vWrapped.size() > 1)
						{
							std::string sLog = "";
							for (size_t i = 1; i < vWrapped.size(); i++)
							{
								sLog += vWrapped[i].c_str();
								if (i != vWrapped.size() - 1)
									sLog += " ";
							}
							vWrapped = WrapText(sLog, flWidth);
							for (size_t i = 0; i < vWrapped.size(); i++)
							{
								FText(vWrapped[i].c_str());
								if (i == vWrapped.size() - 1)
									flTitleWidth = FCalcTextSize(vWrapped[i].c_str()).x + H::Draw.Scale(4);
							}
							iLines += vWrapped.size();
						}
					}
				}
				else
				{
					PushStyleColor(ImGuiCol_Text, ColorToVec(tOutput.tAccent));

					auto vWrapped = WrapText(tOutput.m_sLog, flWidth);
					for (size_t i = 0; i < vWrapped.size(); i++)
						FText(vWrapped[i].c_str());
					iLines = vWrapped.size();

					PopStyleColor();
				}

				SetCursorPos(vOriginalPos); DebugDummy({ flWidth, H::Draw.Scale(13) * iLines + GetStyle().WindowPadding.y });

				if (IsItemHovered() && IsMouseDown(ImGuiMouseButton_Right))
					OpenPopup(std::format("output{}", tOutput.m_iID).c_str());
				if (FBeginPopup(std::format("output{}", tOutput.m_iID).c_str()))
				{
					PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

					if (FSelectable("copy"))
						SDK::SetClipboard(std::format("{}{}{}", tOutput.m_sFunction, tOutput.m_sFunction != "" ? " " : "", tOutput.m_sLog));

					PopStyleVar();
					EndPopup();
				}
			}
		} EndSection();
	}
}

void CMenu::MenuSettings(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
		// Settings
	case 0:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("config"))
			{
				if (FButton("configs folder", FButton_None))
					ShellExecuteA(NULL, NULL, F::Configs.m_sConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

				static int iCurrentType = 0;
				FTabs({ "general", }, &iCurrentType, { GetColumnWidth() / 1, H::Draw.Scale(40) }, { H::Draw.Scale(8), GetCursorPos().y }, false);

				switch (iCurrentType)
				{
					// General
				case 0:
				{
					static std::string newName;
					FSDropdown("config name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
					if (FButton("create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.m_sConfigPath + newName))
							F::Configs.SaveConfig(newName);
						newName.clear();
					}

					std::vector<std::pair<std::filesystem::directory_entry, std::string>> vConfigs = {};
					bool bDefaultFound = false;
					for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sConfigPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
							continue;

						std::string sConfigName = entry.path().filename().string();
						sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());
						if (FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"))
							bDefaultFound = true;

						vConfigs.emplace_back(entry, sConfigName);
					}
					if (!bDefaultFound)
						F::Configs.SaveConfig("default");
					std::sort(vConfigs.begin(), vConfigs.end(), [&](const auto& a, const auto& b) -> bool
						{
							// override for default config
							if (FNV1A::Hash32(a.second.c_str()) == FNV1A::Hash32Const("default"))
								return true;
							if (FNV1A::Hash32(b.second.c_str()) == FNV1A::Hash32Const("default"))
								return false;

							return a.second < b.second;
						});

					for (auto& [entry, sConfigName] : vConfigs)
					{
						bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.m_sCurrentConfig.c_str());
						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

						int iOffset = 0;
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str());

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_SAVE))
						{
							if (!bCurrentConfig || F::Configs.m_sCurrentVisuals.length())
								OpenPopup(std::format("Confirmation## SaveConfig{}", sConfigName).c_str());
							else
								F::Configs.SaveConfig(sConfigName);
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DOWNLOAD))
							F::Configs.LoadConfig(sConfigName);

						if (FBeginPopupModal(std::format("Confirmation## SaveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to override '{}'?", sConfigName).c_str());

							if (FButton("Yes, override", FButton_Left))
							{
								F::Configs.SaveConfig(sConfigName);
								CloseCurrentPopup();
							}
							if (FButton("No", FButton_Right | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}

						if (FBeginPopupModal(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("do you really want to remove '{}' and it's family?", sConfigName).c_str());

							PushDisabled(FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"));
							{
								if (FButton("yes, delete", FButton_Fit))
								{
									F::Configs.RemoveConfig(sConfigName);
									CloseCurrentPopup();
								}
							}
							PopDisabled();
							if (FButton("yes, reset", FButton_Fit | FButton_SameLine))
							{
								F::Configs.ResetConfig(sConfigName);
								CloseCurrentPopup();
							}
							if (FButton("no", FButton_Fit | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}

						SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
					}
					break;
				}
				// Visuals
				case 1:
				{
					static std::string newName;
					FSDropdown("config name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
					if (FButton("create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.m_sVisualsPath + newName))
							F::Configs.SaveVisual(newName);
						newName.clear();
					}

					for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sVisualsPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
							continue;

						std::string sConfigName = entry.path().filename().string();
						sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());

						bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.m_sCurrentVisuals.c_str());
						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

						int iOffset = 0;
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("confirmation## deletevisual{}", sConfigName).c_str());

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_SAVE))
						{
							if (!bCurrentConfig)
								OpenPopup(std::format("confirmation## savevisual{}", sConfigName).c_str());
							else
								F::Configs.SaveVisual(sConfigName);
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DOWNLOAD))
							F::Configs.LoadVisual(sConfigName);

						// Dialogs
						{
							// Save config dialog
							if (FBeginPopupModal(std::format("confirmation## saveVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("do you really want to override '{}' and give it's family a new father?", sConfigName).c_str());

								if (FButton("yes, override", FButton_Left))
								{
									F::Configs.SaveVisual(sConfigName);
									CloseCurrentPopup();
								}
								if (FButton("no", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							// Delete config dialog
							if (FBeginPopupModal(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("Do you really want to delete '{}' and it's newborn son and wife?", sConfigName).c_str());

								if (FButton("yes, delete", FButton_Left))
								{
									F::Configs.RemoveVisual(sConfigName);
									CloseCurrentPopup();
								}
								if (FButton("no", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
					}
				}
				}
			} EndSection();
			SetCursorPosX(GetCursorPosX() + 8);
			//PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			//FText("built " __DATE__);
			//PopStyleColor();

			/* Column 2 */
			TableNextColumn();
			if (Section("other", true))
			{
				FToggle("anti aim lines", Vars::Debug::AntiAimLines, FToggle_Left);
				FToggle("crash logging", Vars::Debug::CrashLogging, FToggle_Right);

				if (FButton("cl_fullupdate", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
				if (FButton("retry", FButton_Right | FButton_SameLine))
					I::EngineClient->ClientCmd_Unrestricted("retry");
				if (FButton("console", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("toggleconsole");
				if (FButton("fix materials", FButton_Right | FButton_SameLine) && F::Materials.m_bLoaded)
					F::Materials.ReloadMaterials();
				if (FButton("itemtest", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("map itemtest");
				if (FButton("tr_walkway", FButton_Right | FButton_SameLine))
					I::EngineClient->ClientCmd_Unrestricted("map workshop/606778917; sv_allow_point_servercommand always");

				if (Vars::Debug::Options.Value && I::EngineClient->IsConnected())
				{
					if (FButton("Restore lines", FButton_Left))
						F::Visuals.RestoreLines();
					if (FButton("Restore paths", FButton_Right | FButton_SameLine))
						F::Visuals.RestorePaths();
					if (FButton("Restore boxes", FButton_Left))
						F::Visuals.RestoreBoxes();
					if (FButton("Clear visuals", FButton_Right | FButton_SameLine))
					{
						G::LineStorage.clear();
						G::PathStorage.clear();
						G::BoxStorage.clear();
					}
				}
			} EndSection();
			if (G::PrivilegedWhiteNigga == true)
			{
				if (Section("debug", true))
				{
					FToggle("debug info", Vars::Debug::Info, FToggle_Left);
					FToggle("show splash points", Vars::Debug::ShowSplashPoints, FToggle_Right);
				} EndSection();
			}
			if (!I::EngineClient->IsConnected())
			{
				if (Section("achievements"))
				{
					if (FButton("unlock achievements"))
						OpenPopup("UnlockAchievements");
					if (FButton("lock achievements"))
						OpenPopup("LockAchievements");

					if (FBeginPopupModal("UnlockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText("do you really want to unlock all achievements?");

						if (FButton("yes, unlock", FButton_Left))
						{
							F::Misc.UnlockAchievements();
							CloseCurrentPopup();
						}
						if (FButton("no", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
					if (FBeginPopupModal("LockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText("do you really want to lock all achievements?");

						if (FButton("yes, lock", FButton_Left))
						{
							F::Misc.LockAchievements();
							CloseCurrentPopup();
						}
						if (FButton("no", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				} EndSection();
			}

			EndTable();
		}
		break;
	case 1:
		if (Section("Players"))
		{
			if (I::EngineClient->IsInGame())
			{
				std::lock_guard lock(F::PlayerUtils.m_mutex);
				const auto& playerCache = F::PlayerUtils.m_vPlayerCache;

				auto getTeamColor = [&](int team, bool alive)
					{
						// TODO: make these team color specified by user.
						switch (team)
						{
						case 3: return Color_t(100, 150, 200, alive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
						case 2: return Color_t(255, 100, 100, alive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
						}
						return Color_t(127, 127, 127, 255).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
					};
				auto drawPlayer = [getTeamColor](const ListPlayer& player, int x, int y)
					{
						ImColor tColor = ColorToVec(getTeamColor(player.m_iTeam, player.m_bAlive));

						ImVec2 vOriginalPos = { !x ? GetStyle().WindowPadding.x : GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(36 + 36 * y) };

						// background
						float flWidth = GetWindowWidth() / 2 - GetStyle().WindowPadding.x * 1.5f;
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						GetWindowDrawList()->AddRect(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(3)); // Changed to outline

						// text + icons
						int lOffset = H::Draw.Scale(10);
						if (player.m_bLocal)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_PERSON);
						}
						else if (F::Spectate.m_iIntendedTarget == player.m_iUserID)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_VISIBILITY);
						}
						SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y + H::Draw.Scale(7) });
						FText(player.m_sName.c_str());
						lOffset += FCalcTextSize(player.m_sName.c_str()).x + H::Draw.Scale(8);

						// buttons
						bool bClicked = false, bAdd = false, bAlias = false, bPitch = false, bYaw = false, bMinwalk = false, bView = false;

						int iOffset = 2;

						if (!player.m_bFake)
						{
							// right
							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bAlias = IconButton(ICON_MD_EDIT);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bAdd = IconButton(ICON_MD_ADD);
						}

						bool bResolver = Vars::AntiHack::Resolver::Enabled.Value && !player.m_bLocal;
						if (bResolver)
						{
							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bView = IconButton(ICON_MD_NEAR_ME);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bMinwalk = IconButton(ICON_MD_DIRECTIONS_WALK);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bYaw = IconButton(ICON_MD_ARROW_FORWARD);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bPitch = IconButton(ICON_MD_ARROW_UPWARD);
						}

						if (!player.m_bFake)
						{
							// tag bar
							SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y });
							if (BeginChild(std::format("TagBar{}", player.m_uFriendsID).c_str(), { flWidth - lOffset - H::Draw.Scale(bResolver ? 128 : 48), flHeight }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								std::vector<std::pair<PriorityLabel_t*, int>> vTags = {};
								if (player.m_bFriend)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)], 0);
								if (player.m_bParty)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)], 0);
								if (player.m_bF2P)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(F2P_TAG)], 0);
								for (auto& iID : F::PlayerUtils.m_mPlayerTags[player.m_uFriendsID])
								{
									auto pTag = F::PlayerUtils.GetTag(iID);
									if (pTag)
										vTags.emplace_back(pTag, iID);
								}

								PushFont(F::Render.FontSmall);
								const auto vDrawPos = GetDrawPos();
								float flTagOffset = 0;
								for (auto& [pTag, iID] : vTags)
								{
									ImColor tTagColor = ColorToVec(pTag->Color);
									float flTagWidth = FCalcTextSize(pTag->Name.c_str()).x + H::Draw.Scale(!iID ? 10 : 25);
									float flTagHeight = H::Draw.Scale(20);
									ImVec2 vTagPos = { flTagOffset, H::Draw.Scale(4) };

									PushStyleColor(ImGuiCol_Text, IsColorDark(tTagColor) ? ImVec4{ 1, 1, 1, 1 } : ImVec4{ 0, 0, 0, 1 });

									GetWindowDrawList()->AddRectFilled(vDrawPos + vTagPos, { vDrawPos.x + vTagPos.x + flTagWidth, vDrawPos.y + vTagPos.y + flTagHeight }, tTagColor, H::Draw.Scale(3));
									SetCursorPos({ vTagPos.x + H::Draw.Scale(5), vTagPos.y + H::Draw.Scale(4) });
									FText(pTag->Name.c_str());
									SetCursorPos({ vTagPos.x + flTagWidth - H::Draw.Scale(18), vTagPos.y + H::Draw.Scale(2) });
									if (iID && IconButton(ICON_MD_CANCEL))
										F::PlayerUtils.RemoveTag(player.m_uFriendsID, iID, true, player.m_sName);

									PopStyleColor();

									flTagOffset += flTagWidth + H::Draw.Scale(4);
								}
								PopFont();
							} EndChild();

							//bClicked = IsItemClicked();
							bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;

							SetCursorPos(vOriginalPos);
							/*bClicked = */Button(std::format("##{}", player.m_sName).c_str(), { flWidth, 28 }) || bClicked;
							bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;
						}

						SetCursorPos(vOriginalPos);
						DebugDummy({ 0, H::Draw.Scale(28) });

						if (bClicked)
							OpenPopup(std::format("Clicked{}", player.m_uFriendsID).c_str());
						else if (bAdd)
							OpenPopup(std::format("Add{}", player.m_uFriendsID).c_str());
						else if (bAlias)
							OpenPopup(std::format("Alias{}", player.m_uFriendsID).c_str());
						else if (bYaw)
							OpenPopup(std::format("Yaw{}", player.m_uFriendsID).c_str());
						else if (bPitch)
							OpenPopup(std::format("Pitch{}", player.m_uFriendsID).c_str());
						else if (bMinwalk)
							OpenPopup(std::format("Minwalk{}", player.m_uFriendsID).c_str());
						else if (bView)
							OpenPopup(std::format("View{}", player.m_uFriendsID).c_str());

						// popups
						if (FBeginPopup(std::format("Clicked{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							if (player.m_iLevel != -2)
							{
								std::string sLevel = "T? L?";
								if (player.m_iLevel != -1)
								{
									int iTier = std::ceil(player.m_iLevel / 150.f);
									int iLevel = ((player.m_iLevel - 1) % 150) + 1;
									sLevel = std::format("T{} L{}", iTier, iLevel);
								}
								PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
								FText(sLevel.c_str());
								PopStyleColor();
							}

							if (FSelectable("Profile"))
								I::SteamFriends->ActivateGameOverlayToUser("steamid", CSteamID(player.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual));

							if (FSelectable("History"))
								I::SteamFriends->ActivateGameOverlayToWebPage(std::format("https://steamhistory.net/id/{}", CSteamID(player.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64()).c_str());

							if (FSelectable(F::Spectate.m_iIntendedTarget == player.m_iUserID ? "Unspectate" : "Spectate"))
								F::Spectate.SetTarget(player.m_iUserID);

							if (!player.m_bLocal && FSelectable("Votekick"))
								I::ClientState->SendStringCmd(std::format("callvote kick {}", player.m_iUserID).c_str());

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Add{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
							{
								int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
								auto& tTag = *it;
								if (!tTag.Assignable || F::PlayerUtils.HasTag(player.m_uFriendsID, iID))
									continue;

								auto imColor = ColorToVec(tTag.Color);
								PushStyleColor(ImGuiCol_Text, imColor);
								imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
								if (FSelectable(tTag.Name.c_str(), imColor))
									F::PlayerUtils.AddTag(player.m_uFriendsID, iID, true, player.m_sName);
								PopStyleColor();
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Alias{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							FText("Alias");

							bool bHasAlias = F::PlayerUtils.m_mPlayerAliases.contains(player.m_uFriendsID);
							static std::string sInput = "";

							PushStyleVar(ImGuiStyleVar_FramePadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
							PushItemWidth(H::Draw.Scale(150));
							bool bEnter = InputText("##Alias", &sInput, ImGuiInputTextFlags_EnterReturnsTrue);
							if (!IsItemFocused())
								sInput = bHasAlias ? F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID] : "";
							PopItemWidth();
							PopStyleVar();

							if (bEnter)
							{
								if (sInput.empty() && F::PlayerUtils.m_mPlayerAliases.contains(player.m_uFriendsID))
								{
									F::Output.AliasChanged(player.m_sName, "Removed", F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID]);

									auto find = F::PlayerUtils.m_mPlayerAliases.find(player.m_uFriendsID);
									if (find != F::PlayerUtils.m_mPlayerAliases.end())
										F::PlayerUtils.m_mPlayerAliases.erase(find);
									F::PlayerUtils.m_bSave = true;
								}
								else
								{
									F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID] = sInput;
									F::PlayerUtils.m_bSave = true;

									F::Output.AliasChanged(player.m_sName, bHasAlias ? "Changed" : "Added", sInput);
								}
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Yaw{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							static std::vector<std::pair<std::string, float>> vYaws = {
								{ "Auto", 0.f },
								{ "Forward", 0.f },
								{ "Left", 90.f },
								{ "Right", -90.f },
								{ "Backwards", 180.f }
							};
							for (auto& [sYaw, flValue] : vYaws)
							{
								if (FSelectable(sYaw.c_str()))
								{
									switch (FNV1A::Hash32(sYaw.c_str()))
									{
									case FNV1A::Hash32Const("Auto"):
										F::Resolver.SetYaw(player.m_iUserID, 0.f, true);
										break;
									default:
										F::Resolver.SetYaw(player.m_iUserID, flValue);
									}
								}
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Pitch{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							static std::vector<std::pair<std::string, float>> vPitches = {
								{ "Auto", 0.f },
								{ "Up", -90.f },
								{ "Down", 90.f },
								{ "Zero", 0.f },
								{ "Inverse", 0.f }
							};
							for (auto& [sPitch, flValue] : vPitches)
							{
								if (FSelectable(sPitch.c_str()))
								{
									switch (FNV1A::Hash32(sPitch.c_str()))
									{
									case FNV1A::Hash32Const("Auto"):
										F::Resolver.SetPitch(player.m_iUserID, 0.f, false, true);
										break;
									case FNV1A::Hash32Const("Inverse"):
										F::Resolver.SetPitch(player.m_iUserID, 0.f, true);
										break;
									default:
										F::Resolver.SetPitch(player.m_iUserID, flValue);
									}
								}
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Minwalk{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							static std::vector<std::pair<std::string, bool>> vPitches = {
								{ "Minwalk on", true },
								{ "Minwalk off", false }
							};
							for (auto& [sPitch, bValue] : vPitches)
							{
								if (FSelectable(sPitch.c_str()))
									F::Resolver.SetMinwalk(player.m_iUserID, bValue);
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("View{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							static std::vector<std::pair<std::string, bool>> vPitches = {
								{ "Offset from static view", true },
								{ "Offset from view to local", false }
							};
							for (auto& [sPitch, bValue] : vPitches)
							{
								if (FSelectable(sPitch.c_str()))
									F::Resolver.SetView(player.m_iUserID, bValue);
							}

							PopStyleVar();
							EndPopup();
						}
					};

				// display players
				std::vector<ListPlayer> vBlu, vRed, vOther;
				for (auto& player : playerCache)
				{
					switch (player.m_iTeam)
					{
					case 3: vBlu.push_back(player); break;
					case 2: vRed.push_back(player); break;
					default: vOther.push_back(player); break;
					}
				}

				if (vBlu.size() < vRed.size()) // display whichever one has more last
				{
					for (size_t i = 0; i < vBlu.size(); i++)
						drawPlayer(vBlu[i], 0, int(i));
					for (size_t i = 0; i < vRed.size(); i++)
						drawPlayer(vRed[i], 1, int(i));
				}
				else
				{
					for (size_t i = 0; i < vRed.size(); i++)
						drawPlayer(vRed[i], 1, int(i));
					for (size_t i = 0; i < vBlu.size(); i++)
						drawPlayer(vBlu[i], 0, int(i));
				}
				size_t iMax = std::max(vBlu.size(), vRed.size());
				for (size_t i = 0; i < vOther.size(); i++)
					drawPlayer(vOther[i], i % 2, int(iMax + i / 2));
			}
			else
			{
				SetCursorPos({ H::Draw.Scale(18), H::Draw.Scale(39) });
				FText("not ingame");
				DebugDummy({ 0, H::Draw.Scale(8) });
			}
		} EndSection();
		if (Section("Tags"))
		{
			static int iID = -1;
			static PriorityLabel_t tTag = {};

			{
				ImVec2 vOriginalPos = GetCursorPos();

				SetCursorPos({ 0, vOriginalPos.y - H::Draw.Scale(8) });
				if (BeginChild("Split1", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &tTag.Name, {}, FDropdown_Left | FSDropdown_AutoUpdate, -10);
					FColorPicker("Color", &tTag.Color, 0, FColorPicker_Dropdown);

					PushDisabled(iID == DEFAULT_TAG || iID == IGNORED_TAG);
					{
						int iLabel = Disabled ? 0 : tTag.Label;
						FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdown_Right);
						tTag.Label = iLabel;
						if (Disabled)
							tTag.Label = false;
					}
					PopDisabled();
				} EndChild();

				SetCursorPos({ GetWindowWidth() / 2 - GetStyle().WindowPadding.x / 2, vOriginalPos.y - H::Draw.Scale(8) });
				if (BeginChild("Split2", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					PushTransparent(tTag.Label); // transparent if we want a label, user can still use to sort
					{
						SetCursorPosY(GetCursorPos().y + H::Draw.Scale(12));
						FSlider("Priority", &tTag.Priority, -10, 10, 1, "%i", FSlider_Left);
					}
					PopTransparent();

					// create/modify button
					bool bCreate = false, bClear = false;

					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(8) });
					PushDisabled(tTag.Name.empty());
					{
						bCreate = FButton("##CreateButton", FButton_None, { 40, 40 });
					}
					PopDisabled();
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(84), H::Draw.Scale(28) });
					PushTransparent(tTag.Name.empty());
					{
						IconImage(iID != -1 ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					PopTransparent();

					// clear button
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(48), H::Draw.Scale(8) });
					bClear = FButton("##ClearButton", FButton_None, { 40, 40 });
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(36), H::Draw.Scale(28) });
					IconImage(ICON_MD_CLEAR);

					if (bCreate)
					{
						F::PlayerUtils.m_bSave = true;
						if (iID > -1 || iID < F::PlayerUtils.m_vTags.size())
						{
							F::PlayerUtils.m_vTags[iID].Name = tTag.Name;
							F::PlayerUtils.m_vTags[iID].Color = tTag.Color;
							F::PlayerUtils.m_vTags[iID].Priority = tTag.Priority;
							F::PlayerUtils.m_vTags[iID].Label = tTag.Label;
						}
						else
							F::PlayerUtils.m_vTags.push_back(tTag);
					}
					if (bCreate || bClear)
					{
						iID = -1;
						tTag = {};
					}
				} EndChild();
			}

			auto drawTag = [](std::vector<PriorityLabel_t>::iterator it, PriorityLabel_t& _tTag, int y)
				{
					int _iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);

					bool bClicked = false, bDelete = false;

					ImVec2 vOriginalPos = { !_tTag.Label ? GetStyle().WindowPadding.x : GetWindowWidth() * 2 / 3 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(96 + 36 * y) };

					// background
					float flWidth = GetWindowWidth() * (_tTag.Label ? 1.f / 3 : 2.f / 3) - GetStyle().WindowPadding.x * 1.5f;
					float flHeight = H::Draw.Scale(28);
					ImColor tColor = ColorToVec(_tTag.Color.Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha));
					ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
					if (iID != _iID)
						GetWindowDrawList()->AddRect(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(3)); // Changed to outline
					else
					{
						ImColor tColor2 = { tColor.Value.x * 1.1f, tColor.Value.y * 1.1f, tColor.Value.z * 1.1f, tColor.Value.w };
						GetWindowDrawList()->AddRect(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor2, H::Draw.Scale(3));

						tColor2 = ColorToVec(_tTag.Color.Lerp(Vars::Menu::Theme::Background.Value, 0.25f, LerpEnum::NoAlpha));
						GetWindowDrawList()->AddRect(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor2, H::Draw.Scale(3), ImDrawFlags_None, H::Draw.Scale());
					}

					// text
					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
					FText(TruncateText(_tTag.Name, _tTag.Label ? flWidth - H::Draw.Scale(38) : flWidth / 2 - H::Draw.Scale(20)).c_str());

					if (!_tTag.Label)
					{
						SetCursorPos({ vOriginalPos.x + flWidth / 2, vOriginalPos.y + H::Draw.Scale(7) });
						FText(std::format("{}", _tTag.Priority).c_str());
					}

					// buttons / icons
					SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(22), vOriginalPos.y + H::Draw.Scale(6) });
					if (!_tTag.Locked)
						bDelete = IconButton(ICON_MD_DELETE);
					else
					{
						switch (F::PlayerUtils.IndexToTag(_iID))
						{
							//case DEFAULT_TAG: // no image
						case IGNORED_TAG: IconImage(ICON_MD_DO_NOT_DISTURB); break;
						case CHEATER_TAG: IconImage(ICON_MD_FLAG); break;
						case FRIEND_TAG: IconImage(ICON_MD_GROUP); break;
						case PARTY_TAG: IconImage(ICON_MD_GROUPS); break;
						case F2P_TAG: IconImage(ICON_MD_MONEY_OFF); break;
						}
					}

					SetCursorPos(vOriginalPos);
					bClicked = Button(std::format("##{}", _tTag.Name).c_str(), { flWidth, flHeight });

					if (bClicked)
					{
						iID = _iID;
						tTag.Name = _tTag.Name;
						tTag.Color = _tTag.Color;
						tTag.Priority = _tTag.Priority;
						tTag.Label = _tTag.Label;
					}
					if (bDelete)
						OpenPopup(std::format("Confirmation## DeleteTag{}", _iID).c_str());
					if (FBeginPopupModal(std::format("Confirmation## DeleteTag{}", _iID).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText(std::format("Do you really want to delete '{}'?", _tTag.Name).c_str());

						if (FButton("Yes", FButton_Left))
						{
							F::PlayerUtils.m_vTags.erase(it);
							F::PlayerUtils.m_bSave = F::PlayerUtils.m_bSave = true;

							for (auto& [friendsID, vTags] : F::PlayerUtils.m_mPlayerTags)
							{
								for (auto it = vTags.begin(); it != vTags.end();)
								{
									if (_iID == *it)
										vTags.erase(it);
									else
									{
										if (_iID < *it)
											(*it)--;
										it++;
									}
								}
							}

							if (iID == _iID)
							{
								iID = -1;
								tTag = {};
							}
							else if (iID > _iID)
								iID--;

							CloseCurrentPopup();
						}
						if (FButton("No", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				};

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ H::Draw.Scale(14), H::Draw.Scale(80) }); FText("Priorities");
			SetCursorPos({ GetWindowWidth() * 2 / 3 + H::Draw.Scale(10), H::Draw.Scale(80) }); FText("Labels");
			PopStyleColor();

			std::vector<std::pair<std::vector<PriorityLabel_t>::iterator, PriorityLabel_t>> vPriorities = {}, vLabels = {};
			for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
			{
				auto& _tTag = *it;

				if (!_tTag.Label)
					vPriorities.emplace_back(it, _tTag);
				else
					vLabels.emplace_back(it, _tTag);
			}

			std::sort(vPriorities.begin(), vPriorities.end(), [&](const auto& a, const auto& b) -> bool
				{
					// override for default tag
					if (std::distance(F::PlayerUtils.m_vTags.begin(), a.first) == DEFAULT_TAG)
						return true;
					if (std::distance(F::PlayerUtils.m_vTags.begin(), b.first) == DEFAULT_TAG)
						return false;

					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.second.Name < b.second.Name;
				});
			std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
				{
					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.second.Name < b.second.Name;
				});

			// display tags
			int iPriorities = 0, iLabels = 0;
			for (auto& pair : vPriorities)
			{
				drawTag(pair.first, pair.second, iPriorities);
				iPriorities++;
			}
			for (auto& pair : vLabels)
			{
				drawTag(pair.first, pair.second, iLabels);
				iLabels++;
			}
			SetCursorPos({ 0, H::Draw.Scale(60 + 36 * std::max(iPriorities, iLabels)) }); DebugDummy({ 0, H::Draw.Scale(28) });
		} EndSection();
		{
			PushDisabled(F::PlayerUtils.m_bLoad);
			{
				ImVec2 vOriginal = GetCursorPos();
				SetCursorPosY(GetCursorPosY() - 8);
				if (FButton("##RefreshButton", FButton_None, { 30, 30 }))
					F::PlayerUtils.m_bLoad = true;
				SetCursorPos(vOriginal + ImVec2(7, 7));
				IconImage(ICON_MD_SYNC);

				SetCursorPos(vOriginal + ImVec2(38, -8));
				if (FButton("Export", FButton_Fit))
				{
					// this should be up2date anyways
					std::ifstream file;
					file.open(F::Configs.m_sCorePath + "Players.by", std::ios_base::app);
					if (file.is_open())
					{
						std::string sString;
						{
							std::string line;
							while (std::getline(file, line))
								sString += line + "\n";
							if (!sString.empty())
								sString.pop_back();
						}
						file.close();

						SDK::SetClipboard(sString);
						SDK::Output("bytespy", "copied playerlist to clipboard!", { 255, 50, 113 }, true, true, true);
					}
				}

				{
					static std::vector<PriorityLabel_t> vTags = {};
					static std::unordered_map<uint32_t, std::vector<int>> mPlayerTags = {};
					static std::unordered_map<uint32_t, std::string> mPlayerAliases = {};
					static std::unordered_map<int, int> mAs = {};

					if (FButton("Import", FButton_Fit | FButton_SameLine))
					{
						try
						{
							// Get Base64-encoded string from clipboard
							std::string base64Input = SDK::GetClipboard();

							// Decode Base64 to JSON string
							std::string decodedJson = Base64Decode(base64Input);

							// Parse JSON from decoded string using boost property_tree
							std::istringstream iss(decodedJson);
							boost::property_tree::ptree readTree;
							boost::property_tree::read_json(iss, readTree);

							mPlayerTags.clear();
							mPlayerAliases.clear();
							mAs.clear();
							vTags = {
								{ "Default", { 200, 200, 200, 255 }, 0, false, false, true },
								{ "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
								{ "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
								{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
								{ "Party", { 100, 100, 255, 255 }, 0, true, false, true },
								{ "F2P", { 255, 255, 255, 255 }, 0, true, false, true }
							};

							if (auto configTree = readTree.get_child_optional("Config"))
							{
								for (auto& it : *configTree)
								{
									PriorityLabel_t tTag = {};
									if (auto getValue = it.second.get_optional<std::string>("Name")) { tTag.Name = *getValue; }
									if (const auto getChild = it.second.get_child_optional("Color")) { F::Configs.TreeToColor(*getChild, tTag.Color); }
									if (auto getValue = it.second.get_optional<int>("Priority")) { tTag.Priority = *getValue; }
									if (auto getValue = it.second.get_optional<bool>("Label")) { tTag.Label = *getValue; }

									int iID = -1;
									try
									{   // new id based indexing
										iID = std::stoi(it.first);
										iID = F::PlayerUtils.TagToIndex(iID);
									}
									catch (...) {}

									if (iID > -1 && iID < vTags.size())
									{
										vTags[iID].Name = tTag.Name;
										vTags[iID].Color = tTag.Color;
										vTags[iID].Priority = tTag.Priority;
										vTags[iID].Label = tTag.Label;
									}
									else
										vTags.push_back(tTag);
								}
							}

							if (auto tagTree = readTree.get_child_optional("Tags"))
							{
								for (auto& player : *tagTree)
								{
									uint32_t friendsID = std::stoi(player.first);

									for (auto& tag : player.second)
									{
										std::string sTag = tag.second.data();

										int iID = -1;
										try
										{   // new id based indexing
											iID = std::stoi(sTag);
											iID = F::PlayerUtils.TagToIndex(iID);
										}
										catch (...) {}
										if (iID == -1)
											continue;

										auto pTag = F::PlayerUtils.GetTag(iID);
										if (!pTag || !pTag->Assignable)
											continue;

										if (!F::PlayerUtils.HasTag(friendsID, iID, mPlayerTags))
											F::PlayerUtils.AddTag(friendsID, iID, false, "", mPlayerTags);
									}
								}
							}

							if (auto aliasTree = readTree.get_child_optional("Aliases"))
							{
								for (auto& player : *aliasTree)
								{
									uint32_t friendsID = std::stoi(player.first);
									std::string sAlias = player.second.data();

									if (!sAlias.empty())
										mPlayerAliases[friendsID] = player.second.data();
								}
							}

							for (int i = 0; i < vTags.size(); i++)
							{
								if (vTags[i].Assignable)
								{
									if (F::PlayerUtils.IndexToTag(i) <= 0)
										mAs[i] = i;
									else
										mAs[i] = -1;
								}
							}
							OpenPopup("Import playerlist");
						}
						catch (...)
						{
							SDK::Output("bytespy", "failed to import playerlist!", { 255, 50, 113 }, true, true, true);
						}
					}

					SetNextWindowSize({ H::Draw.Scale(300), 0 });
					if (FBeginPopupModal("import playerlist", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText("Import");
						FText("As", FText_Right | FText_SameLine);

						for (int i = 0; i < vTags.size(); i++)
						{
							if (!vTags[i].Assignable)
								continue;

							auto& iIDTo = mAs[i];

							ImVec2 vOriginalPos = GetCursorPos();
							PushStyleColor(ImGuiCol_Text, ColorToVec(vTags[i].Color));
							SetCursorPos(vOriginalPos + ImVec2(H::Draw.Scale(8), H::Draw.Scale(5)));
							FText(vTags[i].Name.c_str());
							PopStyleColor();
							SetCursorPos(vOriginalPos - ImVec2(0, H::Draw.Scale(8))); DebugDummy({ GetWindowWidth() - GetStyle().WindowPadding.x * 2, H::Draw.Scale(32) });

							std::vector<const char*> vEntries = { "None" };
							std::vector<int> vValues = { 0 };
							for (int i = 0; i < F::PlayerUtils.m_vTags.size(); i++)
							{
								if (F::PlayerUtils.m_vTags[i].Assignable)
								{
									vEntries.push_back(F::PlayerUtils.m_vTags[i].Name.c_str());
									vValues.push_back(i + 1);
								}
							}
							PushTransparent(iIDTo == -1);
							{
								int iTo = iIDTo + 1;
								FDropdown(std::format("##{}", i).c_str(), &iTo, vEntries, vValues, FSlider_Right);
								iIDTo = iTo - 1;
							}
							PopTransparent();
						}

						if (FButton("Import", FButton_Left))
						{
							for (auto& [friendsID, vTags] : mPlayerTags)
							{
								for (auto& iTag : vTags)
								{
									int iID = mAs.contains(iTag) ? mAs[iTag] : -1;
									if (iID != -1 && !F::PlayerUtils.HasTag(friendsID, iID))
										F::PlayerUtils.AddTag(friendsID, iID, false);
								}
							}
							for (auto& [friendsID, sAlias] : mPlayerAliases)
							{
								if (!F::PlayerUtils.m_mPlayerAliases.contains(friendsID))
									F::PlayerUtils.m_mPlayerAliases[friendsID] = sAlias;
							}

							F::PlayerUtils.m_bSave = true;
							SDK::Output("bytespy", "imported playerlist!", { 255, 50, 113 }, true, true, true);

							CloseCurrentPopup();
						}
						if (FButton("cancel", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				}

				if (FButton("Backup", FButton_Fit | FButton_SameLine))
				{
					try
					{
						int iBackupCount = 0;
						for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sCorePath))
						{
							if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
								continue;

							std::string sConfigName = entry.path().filename().string();
							sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());
							if (sConfigName.find("Backup") != std::string::npos)
								iBackupCount++;
						}
						std::filesystem::copy(
							F::Configs.m_sCorePath + "Players.by",
							F::Configs.m_sCorePath + std::format("Backup{}.by", iBackupCount + 1),
							std::filesystem::copy_options::overwrite_existing
						);
						SDK::Output("bytespy", "saved backup playerlist!", { 255, 50, 113 }, true, true, true);
					}
					catch (...)
					{
						SDK::Output("bytespy", "failed to backup playerlist!", { 255, 50, 113 }, true, true, true);
					}
				}
			}
			PopDisabled();

			if (FButton("Folder", FButton_Fit | FButton_SameLine))
				ShellExecuteA(NULL, NULL, F::Configs.m_sCorePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		break;

	}
}
#pragma endregion

struct DragBoxStorage_t
{
	DragBox_t m_tDragBox;
	float m_flScale;
};
static std::unordered_map<uint32_t, DragBoxStorage_t> mDragBoxStorage = {};
void CMenu::AddDraggable(const char* sLabel, ConfigVar<DragBox_t>& var, bool bShouldDraw, ImVec2 vSize)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	auto tDragBox = FGet(var, true);
	auto uHash = FNV1A::Hash32(sLabel);

	bool bContains = mDragBoxStorage.contains(uHash);
	auto& tStorage = mDragBoxStorage[uHash];

	SetNextWindowSize(vSize, ImGuiCond_Always);
	if (!bContains || tDragBox != tStorage.m_tDragBox || H::Draw.Scale() != tStorage.m_flScale)
		SetNextWindowPos({ float(tDragBox.x - vSize.x / 2), float(tDragBox.y) }, ImGuiCond_Always);

	PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, vSize);
	if (Begin(sLabel, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();
		tDragBox.x = vWindowPos.x + vSize.x / 2;
		tDragBox.y = vWindowPos.y;
		tStorage = { tDragBox, H::Draw.Scale() };
		FSet(var, tDragBox);

		End();
	}
	PopStyleVar(3);
	PopStyleColor(2);
}

struct WindowBoxStorage_t
{
	WindowBox_t m_tWindowBox;
	float m_flScale;
};
static std::unordered_map<uint32_t, WindowBoxStorage_t> mWindowBoxStorage = {};
void CMenu::AddResizableDraggable(const char* sLabel, ConfigVar<WindowBox_t>& var, bool bShouldDraw, ImVec2 vMinSize, ImVec2 vMaxSize, ImGuiSizeCallback fCustomCallback)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	auto tWindowBox = FGet(var, true);
	auto uHash = FNV1A::Hash32(sLabel);

	bool bContains = mWindowBoxStorage.contains(uHash);
	auto& tStorage = mWindowBoxStorage[uHash];

	SetNextWindowSizeConstraints(vMinSize, vMaxSize, fCustomCallback);
	if (!bContains || tWindowBox != tStorage.m_tWindowBox || H::Draw.Scale() != tStorage.m_flScale)
	{
		SetNextWindowPos({ float(tWindowBox.x - tWindowBox.w / 2), float(tWindowBox.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(tWindowBox.w), float(tWindowBox.h) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	if (Begin(sLabel, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();
		ImVec2 vWinSize = GetWindowSize();

		tWindowBox.w = vWinSize.x, tWindowBox.h = vWinSize.y;
		tWindowBox.x = vWindowPos.x + tWindowBox.w / 2, tWindowBox.y = vWindowPos.y;
		tStorage = { tWindowBox, H::Draw.Scale() };
		FSet(var, tWindowBox);

		PushFont(F::Render.FontBold);
		ImVec2 vTextSize = FCalcTextSize(sLabel);
		SetCursorPos({ (vWinSize.x - vTextSize.x) * 0.5f, (vWinSize.y - vTextSize.y) * 0.5f });
		FText(sLabel);
		PopFont();

		End();
	}
	PopStyleVar(2);
	PopStyleColor(2);
}

struct BindInfo_t
{
	const char* sName;
	std::string sInfo;
	std::string sState;

	int iBind;
	Bind_t& tBind;
};

void CMenu::DrawBinds()
{
	using namespace ImGui;

	if (!Vars::Visuals::Misc::ShowBindMenu.Value)
		return;

	if (!m_bIsOpen && !I::EngineClient->IsInGame())
		return;

	// 1) gather binds
	std::vector<BindInfo_t> vInfo;
	std::function<void(int)> getBinds = [&](int iParent)
		{
			for (int i = 0; i < (int)F::Binds.m_vBinds.size(); i++)
			{
				auto& tBind = F::Binds.m_vBinds[i];
				if (iParent != tBind.m_iParent || (!tBind.m_bEnabled && !m_bIsOpen))
					continue;

				if (m_bIsOpen || (tBind.m_bVisible && tBind.m_bActive))
				{
					std::string sType;
					switch (tBind.m_iType)
					{
					case BindEnum::Key:
						switch (tBind.m_iInfo)
						{
						case BindEnum::KeyEnum::Hold:        sType = "held";   break;
						case BindEnum::KeyEnum::Toggle:      sType = "toggled"; break;
						case BindEnum::KeyEnum::DoubleClick: sType = "double"; break;
						}
						break;
					case BindEnum::Class:      sType = "class";  break;
					case BindEnum::WeaponType: sType = "weapon"; break;
					case BindEnum::ItemSlot:   sType = "slot";   break;
					}
					vInfo.emplace_back(tBind.m_sName.c_str(), sType, "", i, tBind);
				}

				if (tBind.m_bActive || m_bIsOpen)
					getBinds(i);
			}
		};
	getBinds(DEFAULT_BIND);

	// 2) position tracking
	static DragBox_t old = { INT_MIN, INT_MIN };
	DragBox_t info = m_bIsOpen
		? FGet(Vars::Menu::BindsDisplay, true)
		: Vars::Menu::BindsDisplay.Value;
	if (info != old)
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);

	// 3) measure text widths
	float flNameWidth = 0, flBracketWidth = 0;
	PushFont(F::Render.FontSmall);
	for (auto& [sName, sType, _, iBind, tBind] : vInfo)
	{
		flNameWidth = std::max(flNameWidth, FCalcTextSize(sName).x);
		std::string bracket = std::format("[{}]", sType);
		std::wstring wB(bracket.begin(), bracket.end());
		int w = 0, h = 0;
		I::MatSystemSurface->GetTextSize(H::Fonts.GetFont(FONT_INDICATORS).m_dwFont, wB.c_str(), w, h);
		flBracketWidth = std::max(flBracketWidth, float(w));
	}
	PopFont();
	flNameWidth += H::Draw.Scale(9);
	flBracketWidth += H::Draw.Scale(9);

	// icons & padding
	float iconSize = H::Draw.Scale(20);
	float padBetween = H::Draw.Scale(5);
	float iconPadding = H::Draw.Scale(10);
	float totalIconW = 4 * iconSize + 2 * padBetween;
	float extraWidth = m_bIsOpen ? iconPadding + totalIconW : 0;

	const float headerH = 20.0f;
	// dynamic width if binds, else we'll clamp to minWidth below
	float flWidth = flNameWidth + flBracketWidth + extraWidth + H::Draw.Scale(14);
	float flHeight = headerH + H::Draw.Scale(18 * vInfo.size()) + H::Draw.Scale(18);

	// 4) reasonable sizing
	PushFont(F::Render.FontLarge);
	const char* hdr = "keybinds";
	float hdrW = CalcTextSize(hdr).x;
	PopFont();
	float minWidth = std::max(hdrW + H::Draw.Scale(20), H::Draw.Scale(60));

	float targetW = vInfo.empty()
		? minWidth
		: std::max(flWidth, minWidth);

	SetNextWindowSize({ targetW, flHeight });
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(40), H::Draw.Scale(40) });

	// Update alpha for each bind
	static std::unordered_map<int, float> bindAlpha;
	float deltaTime = ImGui::GetIO().DeltaTime;
	float fadeSpeed = 5.0f;

	for (auto& [sName, sType, _, iBind, tBind] : vInfo)
	{
		float& alpha = bindAlpha[iBind];
		if (tBind.m_bActive)
		{
			alpha += deltaTime * fadeSpeed;
			if (alpha > 1.0f)
				alpha = 1.0f;
		}
		else
		{
			alpha -= deltaTime * fadeSpeed;
			if (alpha < 0.0f)
				alpha = 0.0f;
		}
	}

	auto lerp = [](const ImVec4& a, const ImVec4& b, float t)
		{
			return ImVec4(
				a.x + (b.x - a.x) * t,
				a.y + (b.y - a.y) * t,
				a.z + (b.z - a.z) * t,
				a.w + (b.w - a.w) * t
			);
		};

	// 5) draw
	if (Begin("Binds", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 pos = GetWindowPos();
		info.x = pos.x; info.y = pos.y;
		old = info;
		if (m_bIsOpen)
			FSet(Vars::Menu::BindsDisplay, info);

		auto* dl = GetWindowDrawList();
		dl->AddRectFilled(pos,
			{ pos.x + targetW, pos.y + headerH },
			ImColor(0, 0, 0, 150), 3.0f);

		// header
		PushFont(F::Render.FontLarge);
		float cent = (targetW - hdrW) * 0.5f;
		SetCursorPos({ cent, 2 });
		Text(hdr);
		PopFont();
		ImU32 accent = ColorConvertFloat4ToU32(F::Render.Accent.Value);
		dl->AddRectFilled(
			{ pos.x, pos.y + headerH - 2 },
			{ pos.x + targetW + 2, pos.y + headerH },
			accent
		);

		// bind entries
		int listY = headerH + H::Draw.Scale(5);
		PushFont(F::Render.FontSmall);
		for (size_t i = 0; i < vInfo.size(); i++)
		{
			auto& [sName, sType, _, iBind, tBind] = vInfo[i];
			float alpha = bindAlpha[iBind];

			float x = H::Draw.Scale(12);
			float y = H::Draw.Scale(listY) + H::Draw.Scale(18) * i;

			// Name color with fade animation
			ImVec4 nameColor = lerp(F::Render.Inactive.Value, F::Render.Accent.Value, alpha);
			SetCursorPos({ x, y });
			PushStyleColor(ImGuiCol_Text, nameColor);
			FText(sName);
			PopStyleColor();

			std::string bracket = std::format("[{}]", sType);
			// Bracket color with fade animation
			ImVec4 bracketColor = lerp(F::Render.Inactive.Value, F::Render.Active.Value, alpha);
			SetCursorPos({ x + flNameWidth, y });
			PushStyleColor(ImGuiCol_Text, bracketColor);
			FText(bracket.c_str());
			PopStyleColor();

			if (m_bIsOpen)
			{
				float iconX = x + flNameWidth + flBracketWidth + iconPadding;
				SetCursorPos({ iconX, y });
				if (IconButton(tBind.m_bEnabled ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF))
					tBind.m_bEnabled = !tBind.m_bEnabled;
				SameLine(0, padBetween);
				if (IconButton(!tBind.m_bVisible ? ICON_MD_VISIBILITY_OFF : ICON_MD_VISIBILITY))
					tBind.m_bVisible = !tBind.m_bVisible;
				SameLine(0, padBetween);
				if (IconButton(!tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF))
					tBind.m_bNot = !tBind.m_bNot;
				SameLine(0, padBetween);
				if (IconButton(ICON_MD_DELETE))
					F::Binds.RemoveBind(iBind);
			}
		}
		PopFont();

		End();
	}
	PopStyleVar();
}

void CMenu::Render()
{
	using namespace ImGui;

	for (int iKey = 0; iKey < 256; iKey++)
		U::KeyHandler.StoreKey(iKey);

	if (!F::Configs.m_bConfigLoaded || !(ImGui::GetIO().DisplaySize.x > 160.f && ImGui::GetIO().DisplaySize.y > 28.f))
		return;

	m_bInKeybind = false;
	if (U::KeyHandler.Pressed(Vars::Menu::MenuPrimaryKey.Value) || U::KeyHandler.Pressed(Vars::Menu::MenuSecondaryKey.Value))
		I::MatSystemSurface->SetCursorAlwaysVisible(m_bIsOpen = !m_bIsOpen);

	PushFont(F::Render.FontRegular);

	DrawBinds();
	if (m_bIsOpen)
	{
		DrawMenu();

		// this code is autism 
		AddDraggable("doubletap", Vars::Menu::TicksDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ticks);
		AddDraggable("crit hack", Vars::Menu::CritsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::CritHack);
		AddDraggable("spectators", Vars::Menu::SpectatorsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Spectators);
		AddDraggable("ping", Vars::Menu::PingDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ping);
		AddDraggable("seed prediction", Vars::Menu::SeedPredictionDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::SeedPrediction);
		AddDraggable("spotify", Vars::Menu::SpotifyDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Spotify);
		AddDraggable("movement recorder", Vars::Menu::MovementRecorderDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::MovementRecorder);
		AddDraggable("conditions", Vars::Menu::ConditionsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Conditions);
		AddResizableDraggable("  ", Vars::Visuals::Simulation::ProjectileWindow, FGet(Vars::Visuals::Simulation::ProjectileCamera));
		//AddResizableDraggable(" ", Vars::Radar::Main::Window, FGet(Vars::Radar::Main::Enabled), { H::Draw.Scale(100), H::Draw.Scale(100) }, { H::Draw.Scale(1000), H::Draw.Scale(1000) });

		F::Render.Cursor = GetMouseCursor();

		if (!vDisabled.empty())
		{
			IM_ASSERT_USER_ERROR(0, "calling PopDisabled() too little times: stack overflow.");
			Disabled = false;
			vDisabled.clear();
		}
		if (!vTransparent.empty())
		{
			IM_ASSERT_USER_ERROR(0, "calling PopTransparent() too little times: stack overflow.");
			Transparent = false;
			vTransparent.clear();
		}
	}
	else
		mActiveMap.clear();

	PopFont();
}

void CMenu::AddOutput(const std::string& sFunction, const std::string& sLog, const Color_t& tColor)
{
	static size_t iID = 0;

	m_vOutput.emplace_back(sFunction, sLog, iID++, tColor);
	while (m_vOutput.size() > m_iMaxOutputSize)
		m_vOutput.pop_front();
}