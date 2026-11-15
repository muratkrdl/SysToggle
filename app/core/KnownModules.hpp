#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "core/ModuleInfo.hpp"

namespace SysToggle::Core
{
    struct KnownModule
    {
        std::uint64_t ProgramId;
        const char* Name;
        bool RequiresReboot;
        ModuleWarning Warning;
    };

    class KnownModules
    {
    public:
        static std::optional<KnownModule> Find(std::uint64_t programId);
    };
}
