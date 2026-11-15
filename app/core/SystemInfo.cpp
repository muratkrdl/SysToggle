#include "core/SystemInfo.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace SysToggle::Core
{
#ifdef __SWITCH__

    std::optional<MemoryUsage> SystemInfo::QuerySystemPool()
    {
        u64 total = 0;
        u64 used = 0;
        bool totalRead = R_SUCCEEDED(svcGetSystemInfo(&total, SystemInfoType_TotalPhysicalMemorySize,
            INVALID_HANDLE, PhysicalMemorySystemInfo_System));
        bool usedRead = R_SUCCEEDED(svcGetSystemInfo(&used, SystemInfoType_UsedPhysicalMemorySize,
            INVALID_HANDLE, PhysicalMemorySystemInfo_System));
        if (!totalRead || !usedRead)
            return std::nullopt;
        return MemoryUsage{ total, used };
    }

    bool SystemInfo::Reboot()
    {
        if (R_FAILED(spsmInitialize()))
            return false;
        Result result = spsmShutdown(true);
        spsmExit();
        return R_SUCCEEDED(result);
    }

#else

    std::optional<MemoryUsage> SystemInfo::QuerySystemPool()
    {
        return std::nullopt;
    }

    bool SystemInfo::Reboot()
    {
        return false;
    }

#endif
}
