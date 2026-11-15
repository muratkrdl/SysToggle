#pragma once

#include <cstdint>
#include <optional>

namespace SysToggle::Core
{
    struct MemoryUsage
    {
        std::uint64_t TotalBytes;
        std::uint64_t UsedBytes;

        std::uint64_t FreeBytes() const { return TotalBytes > UsedBytes ? TotalBytes - UsedBytes : 0; }
    };

    class SystemInfo
    {
    public:
        static std::optional<MemoryUsage> QuerySystemPool();
        static bool Reboot();
    };
}
