#include "PlayerCore.h"
#include "PlayerUtils.h"
#include "../Configs/Configs.h"
#include <fstream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

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
    static std::vector<int> T(256, -1);
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
        initialized = true;
    }

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

void CPlayerlistCore::Run()
{
    static Timer tTimer = {};
    if (!tTimer.Run(1.f))
        return;

    LoadPlayerlist();
    SavePlayerlist();
}

void CPlayerlistCore::SavePlayerlist()
{
    if (!F::PlayerUtils.m_bSave || F::PlayerUtils.m_bLoad) // terrible if we end up saving while loading
        return;

    try
    {
        boost::property_tree::ptree writeTree;

        boost::property_tree::ptree configTree;
        for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
        {
            int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
            auto& tTag = *it;

            boost::property_tree::ptree tagEntry;
            tagEntry.put("Name", tTag.Name);
            tagEntry.put_child("Color", F::Configs.ColorToTree(tTag.Color));
            tagEntry.put("Priority", tTag.Priority);
            tagEntry.put("Label", tTag.Label);

            configTree.put_child(std::to_string(F::PlayerUtils.IndexToTag(iID)), tagEntry);
        }
        writeTree.put_child("Config", configTree);

        boost::property_tree::ptree tagTree;
        for (auto& [friendsID, vTags] : F::PlayerUtils.m_mPlayerTags)
        {
            if (vTags.empty())
                continue;

            boost::property_tree::ptree tagList;
            for (auto& iID : vTags)
            {
                boost::property_tree::ptree child; child.put("", F::PlayerUtils.IndexToTag(iID));
                tagList.push_back(std::make_pair("", child));
            }

            tagTree.put_child(std::to_string(friendsID), tagList);
        }
        writeTree.put_child("Tags", tagTree);

        boost::property_tree::ptree aliasTree;
        for (auto& [friendsID, sAlias] : F::PlayerUtils.m_mPlayerAliases)
        {
            if (sAlias.empty())
                continue;

            aliasTree.put(std::to_string(friendsID), sAlias);
        }
        writeTree.put_child("Aliases", aliasTree);

        // Serialize ptree to JSON string
        std::ostringstream oss;
        boost::property_tree::write_json(oss, writeTree, false);
        std::string json_str = oss.str();

        // Base64 encode
        std::string encoded = Base64Encode(json_str);

        // Write to file
        std::ofstream ofs(F::Configs.m_sCorePath + "Players.by", std::ios::binary);
        ofs << encoded;
        ofs.close();

        F::PlayerUtils.m_bSave = false;
        SDK::Output("bytespy", "saved playerlist!", { 255, 50, 113 }, true, true, true);
    }
    catch (...)
    {
        SDK::Output("SavePlayerlist", "Failed", { 255, 50, 113 }, true, true);
    }
}

void CPlayerlistCore::LoadPlayerlist()
{
    if (!F::PlayerUtils.m_bLoad)
        return;

    try
    {
        std::string filepath = F::Configs.m_sCorePath + "Players.by";

        if (std::filesystem::exists(filepath))
        {
            // Read entire file contents
            std::ifstream ifs(filepath, std::ios::binary);
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string encoded = buffer.str();
            ifs.close();

            // Base64 decode
            std::string decoded = Base64Decode(encoded);

            // Parse JSON from decoded string
            std::istringstream iss(decoded);
            boost::property_tree::ptree readTree;
            boost::property_tree::read_json(iss, readTree);

            F::PlayerUtils.m_mPlayerTags.clear();
            F::PlayerUtils.m_mPlayerAliases.clear();
            F::PlayerUtils.m_vTags = {
                { "Default", { 200, 200, 200, 255 }, 0, false, false, true },
                { "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
                { "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
                { "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
                { "Party", { 100, 50, 255, 255 }, 0, true, false, true },
                { "F2P", { 255, 255, 255, 255 }, 0, true, false, true }
            };

            int iTagsVersion = readTree.get_child_optional("NewTags") ? 1 : 0; // support for old tag savings
            if (auto configTree = readTree.get_child_optional("Config"))
            {
                iTagsVersion = 2;

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

                    if (iID > -1 && iID < F::PlayerUtils.m_vTags.size())
                    {
                        F::PlayerUtils.m_vTags[iID].Name = tTag.Name;
                        F::PlayerUtils.m_vTags[iID].Color = tTag.Color;
                        F::PlayerUtils.m_vTags[iID].Priority = tTag.Priority;
                        F::PlayerUtils.m_vTags[iID].Label = tTag.Label;
                    }
                    else
                        F::PlayerUtils.m_vTags.push_back(tTag);
                }
            }
            else if (iTagsVersion < 2 && std::filesystem::exists(F::Configs.m_sCorePath + "Tags.by"))
            {   // support legacy file
                boost::property_tree::ptree readTree2;
                read_json(F::Configs.m_sCorePath + "Tags.by", readTree2);

                bool bNewTags = bool(readTree2.get_child_optional("NewTags")); // newer system to support adding default tags better

                auto tagTree = readTree2.get_child_optional("Tags");
                if (!tagTree)
                    tagTree = readTree2; // support format w/o tag tree

                for (auto& it : *tagTree)
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
                        if (bNewTags)
                            iID = F::PlayerUtils.TagToIndex(iID);
                        else if (iID > 3)
                            iID += TAG_COUNT - 3;
                    }
                    catch (...)
                    {   // old string based indexing
                        tTag.Name = it.first;
                        iID = F::PlayerUtils.GetTag(it.first);
                    }

                    if (iID > -1 && iID < F::PlayerUtils.m_vTags.size())
                    {
                        F::PlayerUtils.m_vTags[iID].Name = tTag.Name;
                        F::PlayerUtils.m_vTags[iID].Color = tTag.Color;
                        F::PlayerUtils.m_vTags[iID].Priority = tTag.Priority;
                        F::PlayerUtils.m_vTags[iID].Label = tTag.Label;
                    }
                    else
                        F::PlayerUtils.m_vTags.push_back(tTag);
                }
            }

            auto tagTree = readTree.get_child_optional("Tags");
            if (!tagTree)
                tagTree = readTree; // support format w/o tag tree

            for (auto& player : *tagTree)
            {
                uint32_t friendsID = std::stoi(player.first);

                for (auto& tag : player.second)
                {
                    std::string sTag = std::string(tag.first.data()).empty() ? tag.second.data() : tag.first.data(); // account for dumb old format

                    int iID = -1;
                    try
                    {   // new id based indexing
                        iID = std::stoi(sTag);
                        if (iTagsVersion != 0)
                            iID = F::PlayerUtils.TagToIndex(iID);
                        else if (iID > 3)
                            iID += TAG_COUNT - 3;
                    }
                    catch (...)
                    {   // old string based indexing
                        iID = F::PlayerUtils.GetTag(sTag);
                    }

                    if (iID == -1)
                        continue;

                    auto pTag = F::PlayerUtils.GetTag(iID);
                    if (!pTag || !pTag->Assignable)
                        continue;

                    if (!F::PlayerUtils.HasTag(friendsID, iID))
                        F::PlayerUtils.AddTag(friendsID, iID, false);
                }
            }

            if (auto aliasTree = readTree.get_child_optional("Aliases"))
            {
                for (auto& player : *aliasTree)
                {
                    uint32_t friendsID = std::stoi(player.first);
                    std::string sAlias = player.second.data();

                    if (!sAlias.empty())
                        F::PlayerUtils.m_mPlayerAliases[friendsID] = player.second.data();
                }
            }
        }

        F::PlayerUtils.m_bLoad = false;
        SDK::Output("bytespy", "loaded playerlist!", { 255, 50, 113 }, true, true, true);
    }
    catch (...)
    {
        SDK::Output("LoadPlayerlist", "Failed", { 255, 50, 113 }, true, true);
    }
}
