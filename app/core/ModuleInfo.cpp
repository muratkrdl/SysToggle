#include "core/ModuleInfo.hpp"

#include <cstdio>
#include <utility>

namespace SysToggle::Core
{
    namespace
    {
        constexpr int k_ProgramIdTextLength = 16;
    }

    ModuleInfo::ModuleInfo(std::uint64_t programId, std::string name, std::filesystem::path directory,
        bool requiresReboot, ModuleWarning warning)
        : m_ProgramId(programId)
        , m_Name(std::move(name))
        , m_Directory(std::move(directory))
        , m_RequiresReboot(requiresReboot)
        , m_Warning(warning)
    {
    }

    std::string ModuleInfo::ProgramIdText() const
    {
        char buffer[k_ProgramIdTextLength + 1];
        std::snprintf(buffer, sizeof(buffer), "%016llX", static_cast<unsigned long long>(m_ProgramId));
        return buffer;
    }
}
