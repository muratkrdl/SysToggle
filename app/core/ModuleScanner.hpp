#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "core/ModuleInfo.hpp"

namespace SysToggle::Core
{
    struct ToolboxManifest
    {
        std::string Name;
        bool RequiresReboot;
    };

    class ModuleScanner
    {
    public:
        static std::optional<std::vector<ModuleInfo>> Scan(const std::filesystem::path& contentsDirectory);
        static std::optional<std::uint64_t> ParseProgramId(const std::string& text);
        static std::optional<ToolboxManifest> ReadToolbox(const std::filesystem::path& moduleDirectory);

    private:
        static bool IsSysmodule(const std::filesystem::path& moduleDirectory, bool isKnown);
        static std::optional<ModuleInfo> Describe(const std::filesystem::directory_entry& entry);
    };
}
