#include "Configs.h"

#include "../Binds/Binds.h"
#include "../Visuals/Materials/Materials.h"
#include "../Misc/MovementRecorder/MovementRecorder.h"

#include <string>
#include <vector>

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

boost::property_tree::ptree CConfigs::ColorToTree(const Color_t& color)
{
	boost::property_tree::ptree colorTree;
	colorTree.put("r", color.r);
	colorTree.put("g", color.g);
	colorTree.put("b", color.b);
	colorTree.put("a", color.a);

	return colorTree;
}

void CConfigs::TreeToColor(const boost::property_tree::ptree& tree, Color_t& out)
{
	if (auto v = tree.get_optional<byte>("r")) { out.r = *v; }
	if (auto v = tree.get_optional<byte>("g")) { out.g = *v; }
	if (auto v = tree.get_optional<byte>("b")) { out.b = *v; }
	if (auto v = tree.get_optional<byte>("a")) { out.a = *v; }
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, bool bVal)
{
	mapTree.put(sName, bVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, int iVal)
{
	mapTree.put(sName, iVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, float flVal)
{
	mapTree.put(sName, flVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const IntRange_t& irVal)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", irVal.Min);
	rangeTree.put("Max", irVal.Max);

	mapTree.put_child(sName, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const FloatRange_t& frVal)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", frVal.Min);
	rangeTree.put("Max", frVal.Max);

	mapTree.put_child(sName, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::string& sVal)
{
	mapTree.put(sName, sVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::vector<std::pair<std::string, Color_t>>& vVal)
{
	boost::property_tree::ptree vectorTree;
	for (auto& pair : vVal)
	{
		boost::property_tree::ptree materialTree;
		materialTree.put("Material", pair.first);
		materialTree.put_child("Color", ColorToTree(pair.second));
		vectorTree.push_back(std::make_pair("", materialTree));
	}
	mapTree.put_child(sName, vectorTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Color_t& tVal)
{
	mapTree.put_child(sName, ColorToTree(tVal));
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Gradient_t& tVal)
{
	boost::property_tree::ptree gradientTree;
	gradientTree.put_child("StartColor", ColorToTree(tVal.StartColor));
	gradientTree.put_child("EndColor", ColorToTree(tVal.EndColor));

	mapTree.put_child(sName, gradientTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const DragBox_t& tVal)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", tVal.x);
	dragBoxTree.put("y", tVal.y);

	mapTree.put_child(sName, dragBoxTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const WindowBox_t& tVal)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", tVal.x);
	dragBoxTree.put("y", tVal.y);
	dragBoxTree.put("w", tVal.w);
	dragBoxTree.put("h", tVal.h);

	mapTree.put_child(sName, dragBoxTree);
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, bool& bVal)
{
	if (auto getValue = mapTree.get_optional<bool>(sName))
		bVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, int& iVal)
{
	if (auto getValue = mapTree.get_optional<int>(sName))
		iVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, float& flVal)
{
	if (auto getValue = mapTree.get_optional<float>(sName))
		flVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, IntRange_t& irVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { irVal.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { irVal.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, FloatRange_t& frVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { frVal.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { frVal.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::string& sVal)
{
	if (auto getValue = mapTree.get_optional<std::string>(sName))
		sVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::vector<std::pair<std::string, Color_t>>& vVal)
{
	auto getMaterials = [&](std::vector<std::pair<std::string, Color_t>>& val, const boost::optional<boost::property_tree::ptree&> getVector)
		{
			if (!getVector)
				return;

			val.clear();
			for (auto& [_, tree] : *getVector)
			{
				auto getValue = tree.get_optional<std::string>("Material");
				if (!getValue)
					continue;

				std::string sMat = *getValue;
				Color_t tColor;

				if (const auto getChild = tree.get_child_optional("Color"))
					TreeToColor(*getChild, tColor);

				bool bFound = false; // ensure no duplicates are assigned
				for (auto& pair : val)
				{
					if (FNV1A::Hash32(pair.first.c_str()) == FNV1A::Hash32(sMat.c_str()))
					{
						bFound = true;
						break;
					}
				}
				if (bFound)
					continue;

				val.emplace_back(sMat, tColor);
			}

			// remove invalid materials
			for (auto it = val.begin(); it != val.end();)
			{
				auto uHash = FNV1A::Hash32(it->first.c_str());
				if (uHash == FNV1A::Hash32Const("None")
					|| uHash != FNV1A::Hash32Const("original") && !F::Materials.m_mMaterials.contains(uHash))
					it = val.erase(it);
				else
					++it;
			}
		};

	getMaterials(vVal, mapTree.get_child_optional(sName));
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Color_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
		TreeToColor(*getChild, tVal);
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Gradient_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (const auto getStartColor = getChild->get_child_optional("StartColor"))
			TreeToColor(*getStartColor, tVal.StartColor);
		if (const auto endColor = getChild->get_child_optional("EndColor"))
			TreeToColor(*endColor, tVal.EndColor);
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, DragBox_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { tVal.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { tVal.y = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, WindowBox_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { tVal.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { tVal.y = *getValue; }
		if (auto getValue = getChild->get_optional<int>("w")) { tVal.w = *getValue; }
		if (auto getValue = getChild->get_optional<int>("h")) { tVal.h = *getValue; }
	}
}

CConfigs::CConfigs()
{
	m_sConfigPath = std::filesystem::current_path().string() + "\\bytespy\\";
	m_sVisualsPath = m_sConfigPath + "visuals\\";
	m_sCorePath = m_sConfigPath + "core\\";
	m_sMaterialsPath = m_sConfigPath + "materials\\";

	if (!std::filesystem::exists(m_sConfigPath))
		std::filesystem::create_directory(m_sConfigPath);

	if (!std::filesystem::exists(m_sVisualsPath))
		std::filesystem::create_directory(m_sVisualsPath);

	if (!std::filesystem::exists(m_sCorePath))
		std::filesystem::create_directory(m_sCorePath);

	if (!std::filesystem::exists(m_sMaterialsPath))
		std::filesystem::create_directory(m_sMaterialsPath);
}

#define IsType(type) var->m_iType == typeid(type).hash_code()

#define SaveCond(type, tree)\
{\
	boost::property_tree::ptree mapTree;\
	for (auto& [iBind, tValue] : var->As<type>()->Map)\
		SaveJson(mapTree, std::to_string(iBind), tValue);\
	tree.put_child(var->m_sName.c_str(), mapTree);\
}
#define SaveMain(type, tree) if (IsType(type)) SaveCond(type, tree)
#define LoadCond(type, tree)\
{\
	var->As<type>()->Map = { { DEFAULT_BIND, var->As<type>()->Default } };\
	if (const auto mapTree = tree.get_child_optional(var->m_sName.c_str()))\
	{\
		for (auto& it : *mapTree)\
		{\
			if (!bLegacy)\
			{\
				int iBind = std::stoi(it.first);\
				if ((F::Binds.m_vBinds.size() <= iBind || var->As<type>()->m_iFlags & NOBIND) && iBind != DEFAULT_BIND)\
					continue;\
				LoadJson(*mapTree, it.first, var->As<type>()->Map[iBind]);\
			}\
			else\
			{\
				int iBind = -2; /*invalid bind*/ \
				auto uHash = FNV1A::Hash32(it.first.c_str());\
				if (uHash == FNV1A::Hash32Const("default"))\
					iBind = DEFAULT_BIND;\
				else\
				{\
					for (auto it2 = F::Binds.m_vBinds.begin(); it2 != F::Binds.m_vBinds.end(); it2++)\
					{\
						if (uHash == FNV1A::Hash32(it2->m_sName.c_str()))\
						{\
							iBind = std::distance(F::Binds.m_vBinds.begin(), it2);\
							break;\
						}\
					}\
				}\
				if (iBind == -2 || (F::Binds.m_vBinds.size() <= iBind || var->As<type>()->m_iFlags & NOBIND) && iBind != DEFAULT_BIND)\
					continue;\
				LoadJson(*mapTree, it.first, var->As<type>()->Map[iBind]);\
			}\
		}\
	}\
}
#define LoadMain(type, tree) if (IsType(type)) LoadCond(type, tree)

bool CConfigs::SaveConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		boost::property_tree::ptree bindTree;
		for (auto it = F::Binds.m_vBinds.begin(); it != F::Binds.m_vBinds.end(); it++)
		{
			int iID = std::distance(F::Binds.m_vBinds.begin(), it);
			auto& tBind = *it;

			boost::property_tree::ptree bindTree2;
			bindTree2.put("Name", tBind.m_sName);
			bindTree2.put("Type", tBind.m_iType);
			bindTree2.put("Info", tBind.m_iInfo);
			bindTree2.put("Key", tBind.m_iKey);
			bindTree2.put("Enabled", tBind.m_bEnabled);
			bindTree2.put("Visible", tBind.m_bVisible);
			bindTree2.put("Not", tBind.m_bNot);
			bindTree2.put("Active", tBind.m_bActive);
			bindTree2.put("Parent", tBind.m_iParent);

			bindTree.put_child(std::to_string(iID), bindTree2);
		}
		writeTree.put_child("Binds", bindTree);

		boost::property_tree::ptree varTree;
		for (auto& var : g_Vars)
		{
			if (!bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			SaveMain(bool, varTree)
			else SaveMain(int, varTree)
else SaveMain(float, varTree)
else SaveMain(IntRange_t, varTree)
			else SaveMain(FloatRange_t, varTree)
			else SaveMain(std::string, varTree)
			else SaveMain(VA_LIST(std::vector<std::pair<std::string, Color_t>>), varTree)
			else SaveMain(Color_t, varTree)
			else SaveMain(Gradient_t, varTree)
			else SaveMain(DragBox_t, varTree)
			else SaveMain(WindowBox_t, varTree)
		}
		writeTree.put_child("ConVars", varTree);

		std::ostringstream oss;
		write_json(oss, writeTree);
		std::string encoded = Base64Encode(oss.str());

		std::ofstream out(m_sConfigPath + sConfigName + m_sConfigExtension, std::ios::binary);
		out << encoded;
		out.close();

		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("bytespy", std::format("config {} saved!", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("SaveConfig", "Failed", { 255, 50, 113 }, true, true);
		return false;
	}

	return true;
}

bool CConfigs::LoadConfig(const std::string& sConfigName, bool bNotify)
{
	const auto filePath = m_sConfigPath + sConfigName + m_sConfigExtension;

	if (!std::filesystem::exists(filePath))
	{
		if (sConfigName == std::string("default"))
			SaveConfig("default", false);
		return false;
	}

	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		// read Base64 file
		std::ifstream in(filePath, std::ios::binary);
		std::stringstream buffer;
		buffer << in.rdbuf();
		in.close();

		std::string encoded = buffer.str();
		std::string decoded = Base64Decode(encoded);

		std::istringstream iss(decoded);
		boost::property_tree::ptree readTree;
		read_json(iss, readTree);

		bool bLegacy = false;
		if (const auto condTree = readTree.get_child_optional("Binds"))
		{
			F::Binds.m_vBinds.clear();

			for (auto& it : *condTree)
			{
				Bind_t tBind = {};
				if (auto getValue = it.second.get_optional<std::string>("Name")) { tBind.m_sName = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.m_iType = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.m_iInfo = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.m_iKey = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Enabled")) { tBind.m_bEnabled = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.m_bVisible = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.m_bNot = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.m_bActive = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Parent")) { tBind.m_iParent = *getValue; }

				F::Binds.m_vBinds.push_back(tBind);
			}
		}
		else if (const auto condTree = readTree.get_child_optional("Conditions"))
		{
			bLegacy = true;
			F::Binds.m_vBinds.clear();

			for (auto& it : *condTree)
			{
				if (FNV1A::Hash32(it.first.c_str()) == FNV1A::Hash32Const("default"))
					continue;

				Bind_t tBind = { it.first };
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.m_iType = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.m_iInfo = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.m_iKey = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.m_bVisible = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.m_bNot = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.m_bActive = *getValue; }
				if (auto getValue = it.second.get_optional<std::string>("Parent"))
				{
					auto uHash = FNV1A::Hash32(getValue->c_str());
					for (auto it = F::Binds.m_vBinds.begin(); it != F::Binds.m_vBinds.end(); it++)
					{
						if (FNV1A::Hash32(it->m_sName.c_str()) == uHash)
						{
							tBind.m_iParent = std::distance(F::Binds.m_vBinds.begin(), it);
							break;
						}
					}
				}

				F::Binds.m_vBinds.push_back(tBind);
			}
		}

		if (const auto conVars = readTree.get_child_optional("ConVars"))
		{
			auto& varTree = *conVars;
			for (auto& var : g_Vars)
			{
				if (!bLoadNosave && var->m_iFlags & NOSAVE)
					continue;

				LoadMain(bool, varTree)
				else LoadMain(int, varTree)
		else LoadMain(float, varTree)
		else LoadMain(IntRange_t, varTree)
	else LoadMain(FloatRange_t, varTree)
else LoadMain(std::string, varTree)
				else LoadMain(VA_LIST(std::vector<std::pair<std::string, Color_t>>), varTree)
				else LoadMain(Color_t, varTree)
				else LoadMain(Gradient_t, varTree)
				else LoadMain(DragBox_t, varTree)
				else LoadMain(WindowBox_t, varTree)
			}
		}

		H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);

		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("bytespy", std::format("config {} loaded!", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("LoadConfig", "Failed", { 255, 50, 113 }, true, true);
		return false;
	}

	return true;
}

bool CConfigs::SaveMovementShid(const std::string& configName)
{
	try
	{
		boost::property_tree::ptree root;

		// Save start position
		boost::property_tree::ptree startPosTree;
		startPosTree.put("x", F::MovementRecorder.m_vStartPosition.x);
		startPosTree.put("y", F::MovementRecorder.m_vStartPosition.y);
		startPosTree.put("z", F::MovementRecorder.m_vStartPosition.z);
		root.put_child("StartPosition", startPosTree);

		// Save recorded frames
		boost::property_tree::ptree framesTree;
		for (const auto& frame : F::MovementRecorder.m_vFrames)
		{
			boost::property_tree::ptree frameTree;

			frameTree.put("PosX", frame.vPosition.x);
			frameTree.put("PosY", frame.vPosition.y);
			frameTree.put("PosZ", frame.vPosition.z);

			frameTree.put("ViewX", frame.viewAngles.x);
			frameTree.put("ViewY", frame.viewAngles.y);
			frameTree.put("ViewZ", frame.viewAngles.z);

			frameTree.put("ForwardMove", frame.forwardMove);
			frameTree.put("SideMove", frame.sideMove);
			frameTree.put("UpMove", frame.upMove);

			frameTree.put("Buttons", frame.buttons);
			frameTree.put("TimeStamp", frame.timeStamp);
			frameTree.put("RelativeTick", frame.iRelativeTick);

			framesTree.push_back(std::make_pair("", frameTree));
		}
		root.put_child("RecordedFrames", framesTree);

		// Serialize to JSON
		std::ostringstream oss;
		boost::property_tree::write_json(oss, root);
		std::string jsonStr = oss.str();

		// Base64 encode
		std::string encoded = Base64Encode(jsonStr);

		// Save to file using full path
		const std::string fullPath = F::Configs.m_sConfigPath + configName + ".mvr";
		std::ofstream outFile(fullPath, std::ios::binary);
		outFile << encoded;
		outFile.close();

		// Update currently loaded config (optional)
		F::Configs.m_sCurrentConfig = configName;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CConfigs::LoadMovementShid(const std::string& configName)
{
	try
	{
		const std::string fullPath = F::Configs.m_sConfigPath + configName + ".mvr";

		// Read from file
		std::ifstream inFile(fullPath, std::ios::binary);
		if (!inFile)
			return false;

		std::stringstream buffer;
		buffer << inFile.rdbuf();
		inFile.close();

		// Decode base64
		std::string decoded = Base64Decode(buffer.str());

		// Parse JSON
		std::istringstream iss(decoded);
		boost::property_tree::ptree root;
		boost::property_tree::read_json(iss, root);

		// Load start position
		auto startPosTree = root.get_child("StartPosition");
		F::MovementRecorder.m_vStartPosition.x = startPosTree.get<float>("x");
		F::MovementRecorder.m_vStartPosition.y = startPosTree.get<float>("y");
		F::MovementRecorder.m_vStartPosition.z = startPosTree.get<float>("z");

		// Load recorded frames
		F::MovementRecorder.m_vFrames.clear();
		for (auto& item : root.get_child("RecordedFrames"))
		{
			RecordedFrame_t frame;

			auto& frameTree = item.second;
			frame.vPosition.x = frameTree.get<float>("PosX");
			frame.vPosition.y = frameTree.get<float>("PosY");
			frame.vPosition.z = frameTree.get<float>("PosZ");

			frame.viewAngles.x = frameTree.get<float>("ViewX");
			frame.viewAngles.y = frameTree.get<float>("ViewY");
			frame.viewAngles.z = frameTree.get<float>("ViewZ");

			frame.forwardMove = frameTree.get<float>("ForwardMove");
			frame.sideMove = frameTree.get<float>("SideMove");
			frame.upMove = frameTree.get<float>("UpMove");

			frame.buttons = frameTree.get<int>("Buttons");
			frame.timeStamp = frameTree.get<float>("TimeStamp");
			frame.iRelativeTick = frameTree.get<int>("RelativeTick");

			F::MovementRecorder.m_vFrames.push_back(frame);
		}

		// Update currently loaded config (optional)
		F::Configs.m_sCurrentConfig = configName;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CConfigs::RemoveConfigShid(const std::string& configName)
{
	try
	{
		const std::string fullPath = F::Configs.m_sConfigPath + configName + ".mvr";

		if (std::remove(fullPath.c_str()) != 0)
		{
			return false;
		}

		if (F::Configs.m_sCurrentConfig == configName)
		{
			F::Configs.m_sCurrentConfig.clear();
		}

		return true;
	}
	catch (...)
	{
		return false;
	}
}

#define SaveRegular(type, tree) SaveJson(tree, var->m_sName.c_str(), var->As<type>()->Map[DEFAULT_BIND])
#define SaveMisc(type, tree) if (IsType(type)) SaveRegular(type, tree);
#define LoadRegular(type, tree) LoadJson(tree, var->m_sName.c_str(), var->As<type>()->Map[DEFAULT_BIND])
#define LoadMisc(type, tree) if (IsType(type)) LoadRegular(type, tree);

bool CConfigs::SaveVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		for (auto& var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || !bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			SaveMisc(bool, writeTree)
			else SaveMisc(int, writeTree)
			else SaveMisc(float, writeTree)
			else SaveMisc(IntRange_t, writeTree)
			else SaveMisc(FloatRange_t, writeTree)
			else SaveMisc(std::string, writeTree)
			else SaveMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), writeTree)
			else SaveMisc(Color_t, writeTree)
			else SaveMisc(Gradient_t, writeTree)
			else SaveMisc(DragBox_t, writeTree)
			else SaveMisc(WindowBox_t, writeTree)
		}

		write_json(m_sVisualsPath + sConfigName + m_sConfigExtension, writeTree);
		if (bNotify)
			SDK::Output("bytespy", std::format("visual config {} saved", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("SaveVisual", "Failed", { 255, 50, 113 }, true, true);
		return false;
	}
	return true;
}

bool CConfigs::LoadVisual(const std::string& sConfigName, bool bNotify)
{
	// Check if the visual config exists
	if (!std::filesystem::exists(m_sVisualsPath + sConfigName + m_sConfigExtension))
	{
		//if (sConfigName == std::string("default"))
		//	SaveVisual("default");
		return false;
	}

	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree readTree;
		read_json(m_sVisualsPath + sConfigName + m_sConfigExtension, readTree);

		for (auto& var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || !bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			LoadMisc(bool, readTree)
			else LoadMisc(int, readTree)
			else LoadMisc(float, readTree)
			else LoadMisc(IntRange_t, readTree)
			else LoadMisc(FloatRange_t, readTree)
			else LoadMisc(std::string, readTree)
			else LoadMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), readTree)
			else LoadMisc(Color_t, readTree)
			else LoadMisc(Gradient_t, readTree)
			else LoadMisc(DragBox_t, readTree)
			else LoadMisc(WindowBox_t, readTree)
		}

		m_sCurrentVisuals = sConfigName;
		if (bNotify)
			SDK::Output("bytespy", std::format("Visual config {} loaded!", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("LoadVisual", "Failed", { 255, 50, 113 }, true, true);
		return false;
	}
	return true;
}

#define ResetType(type) var->As<type>()->Map = { { DEFAULT_BIND, var->As<type>()->Default } };
#define ResetT(type) if (IsType(type)) ResetType(type)

void CConfigs::RemoveConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		if (FNV1A::Hash32(sConfigName.c_str()) != FNV1A::Hash32Const("default"))
		{
			std::filesystem::remove(m_sConfigPath + sConfigName + m_sConfigExtension);

			LoadConfig("default", false);

			if (bNotify)
				SDK::Output("bytespy", std::format("Config {} deleted", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
		}
		else
			ResetConfig(sConfigName);
	}
	catch (...)
	{
		SDK::Output("RemoveConfig", "Failed", { 255, 50, 113 }, true, true);
	}
}

void CConfigs::RemoveVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		std::filesystem::remove(m_sVisualsPath + sConfigName + m_sConfigExtension);

		if (bNotify)
			SDK::Output("bytespy", std::format("Visual config {} deleted", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("RemoveVisual", "Failed", { 255, 50, 113 }, true, true);
	}
}

void CConfigs::ResetConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		F::Binds.m_vBinds.clear();

		for (auto& var : g_Vars)
		{
			if (!bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			ResetT(bool)
			else ResetT(int)
			else ResetT(float)
			else ResetT(IntRange_t)
			else ResetT(FloatRange_t)
			else ResetT(std::string)
			else ResetT(std::vector<std::string>)
			else ResetT(Color_t)
			else ResetT(Gradient_t)
			else ResetT(DragBox_t)
			else ResetT(WindowBox_t)
		}

		SaveConfig(sConfigName, false);

		if (bNotify)
			SDK::Output("bytespy", std::format("config {} reset!", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("ResetConfig", "Failed", { 255, 50, 113 }, true, true);
	}
}

void CConfigs::ResetVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		for (auto& var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || !bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			ResetT(bool)
			else ResetT(int)
			else ResetT(float)
			else ResetT(IntRange_t)
			else ResetT(FloatRange_t)
			else ResetT(std::string)
			else ResetT(std::vector<std::string>)
			else ResetT(Color_t)
			else ResetT(Gradient_t)
			else ResetT(DragBox_t)
			else ResetT(WindowBox_t)
		}

		SaveVisual(sConfigName, false);

		if (bNotify)
			SDK::Output("bytespy", std::format("visual config {} reset!", sConfigName).c_str(), { 255, 50, 113 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("ResetVisual", "Failed", { 255, 50, 113 }, true, true);
	}
}