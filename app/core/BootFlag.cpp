#include "core/BootFlag.hpp"

#include <fstream>
#include <system_error>

namespace SysToggle::Core
{
    namespace
    {
        constexpr const char* k_FlagsDirectory = "flags";
        constexpr const char* k_BootFlagFile = "boot2.flag";
    }

    std::filesystem::path BootFlag::FlagPath(const std::filesystem::path& moduleDirectory)
    {
        return moduleDirectory / k_FlagsDirectory / k_BootFlagFile;
    }

    bool BootFlag::IsEnabled(const std::filesystem::path& moduleDirectory)
    {
        std::error_code error;
        return std::filesystem::exists(FlagPath(moduleDirectory), error);
    }

    bool BootFlag::Enable(const std::filesystem::path& moduleDirectory)
    {
        std::error_code error;
        std::filesystem::create_directories(moduleDirectory / k_FlagsDirectory, error);
        if (error)
            return false;
        std::ofstream flag(FlagPath(moduleDirectory), std::ios::binary);
        flag.close();
        return IsEnabled(moduleDirectory);
    }

    bool BootFlag::Disable(const std::filesystem::path& moduleDirectory)
    {
        std::error_code error;
        std::filesystem::remove(FlagPath(moduleDirectory), error);
        return !error && !IsEnabled(moduleDirectory);
    }

    bool BootFlag::Set(const std::filesystem::path& moduleDirectory, bool enabled)
    {
        return enabled ? Enable(moduleDirectory) : Disable(moduleDirectory);
    }
}
