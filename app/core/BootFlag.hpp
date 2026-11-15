#pragma once

#include <filesystem>

namespace SysToggle::Core
{
    class BootFlag
    {
    public:
        static bool IsEnabled(const std::filesystem::path& moduleDirectory);
        static bool Enable(const std::filesystem::path& moduleDirectory);
        static bool Disable(const std::filesystem::path& moduleDirectory);
        static bool Set(const std::filesystem::path& moduleDirectory, bool enabled);

    private:
        static std::filesystem::path FlagPath(const std::filesystem::path& moduleDirectory);
    };
}
