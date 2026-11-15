#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "core/ModuleInfo.hpp"

namespace SysToggle::Core
{
    class Profile
    {
    public:
        Profile(std::string name, std::map<std::uint64_t, bool> bootStates);

        static Profile Capture(const std::string& name, const std::vector<ModuleInfo>& modules);

        const std::string& Name() const { return m_Name; }
        const std::map<std::uint64_t, bool>& BootStates() const { return m_BootStates; }
        std::size_t EnabledCount() const;

    private:
        std::string m_Name;
        std::map<std::uint64_t, bool> m_BootStates;
    };

    struct ApplyResult
    {
        std::size_t Changed;
        std::size_t Unchanged;
        std::size_t Missing;
        std::size_t Failed;

        bool Succeeded() const { return Failed == 0; }
    };

    class ProfileApplier
    {
    public:
        static ApplyResult Apply(const Profile& profile, const std::vector<ModuleInfo>& installedModules);
    };

    class ProfileStore
    {
    public:
        explicit ProfileStore(std::filesystem::path file);

        const std::vector<Profile>& Profiles() const { return m_Profiles; }

        bool Load();
        bool Save() const;
        void Upsert(Profile profile);
        bool Remove(const std::string& name);
        std::optional<std::size_t> IndexOf(const std::string& name) const;

    private:
        std::filesystem::path m_File;
        std::vector<Profile> m_Profiles;
    };
}
