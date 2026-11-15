#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace SysToggle::Core
{
    enum class ModuleWarning
    {
        None,
        SignaturePatches,
        OverlayLoader,
    };

    class ModuleInfo
    {
    public:
        ModuleInfo(std::uint64_t programId, std::string name, std::filesystem::path directory,
            bool requiresReboot, ModuleWarning warning);

        std::uint64_t ProgramId() const { return m_ProgramId; }
        const std::string& Name() const { return m_Name; }
        const std::filesystem::path& Directory() const { return m_Directory; }
        bool RequiresReboot() const { return m_RequiresReboot; }
        ModuleWarning Warning() const { return m_Warning; }
        std::string ProgramIdText() const;

    private:
        std::uint64_t m_ProgramId;
        std::string m_Name;
        std::filesystem::path m_Directory;
        bool m_RequiresReboot;
        ModuleWarning m_Warning;
    };
}
