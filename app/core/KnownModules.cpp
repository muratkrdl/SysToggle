#include "core/KnownModules.hpp"

#include <array>

namespace SysToggle::Core
{
    namespace
    {
        constexpr std::array<KnownModule, 9> k_Modules{{
            { 0x00FF0000636C6BFFull, "sys-clk", false, ModuleWarning::None },
            { 0x420000000007E51Aull, "nx-ovlloader", true, ModuleWarning::OverlayLoader },
            { 0x420000000000000Bull, "sys-patch", true, ModuleWarning::SignaturePatches },
            { 0x010000000000BD00ull, "MissionControl", true, ModuleWarning::None },
            { 0x0000000000534C56ull, "SaltyNX", true, ModuleWarning::None },
            { 0x4200000000000010ull, "ldn_mitm", true, ModuleWarning::None },
            { 0x0100000000000352ull, "emuiibo", false, ModuleWarning::None },
            { 0x690000000000000Dull, "sys-con", false, ModuleWarning::None },
            { 0x420000000000000Eull, "sys-ftpd", false, ModuleWarning::None },
        }};
    }

    std::optional<KnownModule> KnownModules::Find(std::uint64_t programId)
    {
        for (const KnownModule& module : k_Modules)
        {
            if (module.ProgramId == programId)
                return module;
        }
        return std::nullopt;
    }
}
