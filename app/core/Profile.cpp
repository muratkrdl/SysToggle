#include "core/Profile.hpp"

#include <algorithm>
#include <fstream>
#include <system_error>
#include <utility>

#include <borealis/extern/nlohmann/json.hpp>

#include "core/BootFlag.hpp"
#include "core/ModuleScanner.hpp"

namespace SysToggle::Core
{
    namespace
    {
        constexpr const char* k_ProfilesKey = "profiles";
        constexpr const char* k_NameKey = "name";
        constexpr const char* k_ModulesKey = "modules";
        constexpr int k_JsonIndent = 2;

        const ModuleInfo* FindInstalled(const std::vector<ModuleInfo>& modules, std::uint64_t programId)
        {
            for (const ModuleInfo& module : modules)
            {
                if (module.ProgramId() == programId)
                    return &module;
            }
            return nullptr;
        }

        std::optional<Profile> ParseProfile(const nlohmann::json& json)
        {
            if (!json.is_object() || !json.contains(k_NameKey) || !json[k_NameKey].is_string())
                return std::nullopt;
            std::map<std::uint64_t, bool> states;
            if (json.contains(k_ModulesKey) && json[k_ModulesKey].is_object())
            {
                for (const auto& [idText, enabled] : json[k_ModulesKey].items())
                {
                    std::optional<std::uint64_t> programId = ModuleScanner::ParseProgramId(idText);
                    if (programId && enabled.is_boolean())
                        states[*programId] = enabled.get<bool>();
                }
            }
            return Profile(json[k_NameKey].get<std::string>(), std::move(states));
        }

        nlohmann::json SerializeProfile(const Profile& profile)
        {
            nlohmann::json modules = nlohmann::json::object();
            for (const auto& [programId, enabled] : profile.BootStates())
            {
                ModuleInfo idOnly(programId, std::string(), std::filesystem::path(), false, ModuleWarning::None);
                modules[idOnly.ProgramIdText()] = enabled;
            }
            return nlohmann::json{ { k_NameKey, profile.Name() }, { k_ModulesKey, modules } };
        }
    }

    Profile::Profile(std::string name, std::map<std::uint64_t, bool> bootStates)
        : m_Name(std::move(name))
        , m_BootStates(std::move(bootStates))
    {
    }

    Profile Profile::Capture(const std::string& name, const std::vector<ModuleInfo>& modules)
    {
        std::map<std::uint64_t, bool> states;
        for (const ModuleInfo& module : modules)
            states[module.ProgramId()] = BootFlag::IsEnabled(module.Directory());
        return Profile(name, std::move(states));
    }

    std::size_t Profile::EnabledCount() const
    {
        return static_cast<std::size_t>(std::count_if(m_BootStates.begin(), m_BootStates.end(),
            [](const auto& entry) { return entry.second; }));
    }

    ApplyResult ProfileApplier::Apply(const Profile& profile, const std::vector<ModuleInfo>& installedModules)
    {
        ApplyResult result{ 0, 0, 0, 0 };
        for (const auto& [programId, enabled] : profile.BootStates())
        {
            const ModuleInfo* module = FindInstalled(installedModules, programId);
            if (module == nullptr)
            {
                ++result.Missing;
                continue;
            }
            if (BootFlag::IsEnabled(module->Directory()) == enabled)
            {
                ++result.Unchanged;
                continue;
            }
            if (BootFlag::Set(module->Directory(), enabled))
                ++result.Changed;
            else
                ++result.Failed;
        }
        return result;
    }

    ProfileStore::ProfileStore(std::filesystem::path file)
        : m_File(std::move(file))
    {
    }

    bool ProfileStore::Load()
    {
        m_Profiles.clear();
        std::ifstream stream(m_File);
        if (!stream)
            return false;
        nlohmann::json json = nlohmann::json::parse(stream, nullptr, false);
        if (json.is_discarded() || !json.is_object() || !json.contains(k_ProfilesKey)
            || !json[k_ProfilesKey].is_array())
            return false;
        for (const nlohmann::json& entry : json[k_ProfilesKey])
        {
            std::optional<Profile> profile = ParseProfile(entry);
            if (profile)
                m_Profiles.push_back(std::move(*profile));
        }
        return true;
    }

    bool ProfileStore::Save() const
    {
        std::error_code error;
        std::filesystem::create_directories(m_File.parent_path(), error);
        if (error)
            return false;
        nlohmann::json profiles = nlohmann::json::array();
        for (const Profile& profile : m_Profiles)
            profiles.push_back(SerializeProfile(profile));
        std::ofstream stream(m_File, std::ios::trunc);
        if (!stream)
            return false;
        stream << nlohmann::json{ { k_ProfilesKey, profiles } }.dump(k_JsonIndent);
        return static_cast<bool>(stream);
    }

    void ProfileStore::Upsert(Profile profile)
    {
        std::optional<std::size_t> index = IndexOf(profile.Name());
        if (index)
            m_Profiles[*index] = std::move(profile);
        else
            m_Profiles.push_back(std::move(profile));
    }

    bool ProfileStore::Remove(const std::string& name)
    {
        std::optional<std::size_t> index = IndexOf(name);
        if (!index)
            return false;
        m_Profiles.erase(m_Profiles.begin() + static_cast<std::ptrdiff_t>(*index));
        return true;
    }

    std::optional<std::size_t> ProfileStore::IndexOf(const std::string& name) const
    {
        for (std::size_t i = 0; i < m_Profiles.size(); ++i)
        {
            if (m_Profiles[i].Name() == name)
                return i;
        }
        return std::nullopt;
    }
}
