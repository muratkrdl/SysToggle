#include "core/ModuleScanner.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <system_error>

#include <borealis/extern/nlohmann/json.hpp>

#include "core/KnownModules.hpp"
#include "core/NpdmReader.hpp"

namespace SysToggle::Core
{
    namespace
    {
        constexpr std::size_t k_ProgramIdLength = 16;
        constexpr int k_HexBase = 16;
        constexpr const char* k_ExefsFile = "exefs.nsp";
        constexpr const char* k_ExefsDirectory = "exefs";
        constexpr const char* k_NpdmFile = "main.npdm";
        constexpr const char* k_FlagsDirectory = "flags";
        constexpr const char* k_ToolboxFile = "toolbox.json";

        std::string ToLower(std::string text)
        {
            std::transform(text.begin(), text.end(), text.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }
    }

    std::optional<std::uint64_t> ModuleScanner::ParseProgramId(const std::string& text)
    {
        if (text.size() != k_ProgramIdLength)
            return std::nullopt;
        bool allHex = std::all_of(text.begin(), text.end(),
            [](unsigned char c) { return std::isxdigit(c) != 0; });
        if (!allHex)
            return std::nullopt;
        return std::stoull(text, nullptr, k_HexBase);
    }

    std::optional<ToolboxManifest> ModuleScanner::ReadToolbox(const std::filesystem::path& moduleDirectory)
    {
        std::ifstream file(moduleDirectory / k_ToolboxFile);
        if (!file)
            return std::nullopt;
        nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
        if (json.is_discarded() || !json.is_object())
            return std::nullopt;
        ToolboxManifest manifest;
        manifest.Name = json.value("name", std::string());
        manifest.RequiresReboot = json.value("requires_reboot", false);
        return manifest;
    }

    bool ModuleScanner::IsSysmodule(const std::filesystem::path& moduleDirectory, bool isKnown)
    {
        std::error_code error;
        if (std::filesystem::is_regular_file(moduleDirectory / k_ExefsFile, error))
            return true;
        bool hasExefsDirectory = std::filesystem::is_regular_file(
            moduleDirectory / k_ExefsDirectory / k_NpdmFile, error);
        bool looksLikeSysmodule = isKnown
            || std::filesystem::exists(moduleDirectory / k_ToolboxFile, error)
            || std::filesystem::is_directory(moduleDirectory / k_FlagsDirectory, error);
        return hasExefsDirectory && looksLikeSysmodule;
    }

    std::optional<ModuleInfo> ModuleScanner::Describe(const std::filesystem::directory_entry& entry)
    {
        std::error_code error;
        if (!entry.is_directory(error))
            return std::nullopt;
        std::optional<std::uint64_t> programId = ParseProgramId(entry.path().filename().string());
        if (!programId)
            return std::nullopt;
        std::optional<KnownModule> known = KnownModules::Find(*programId);
        if (!IsSysmodule(entry.path(), known.has_value()))
            return std::nullopt;

        std::optional<ToolboxManifest> toolbox = ReadToolbox(entry.path());

        std::string name = entry.path().filename().string();
        if (toolbox && !toolbox->Name.empty())
            name = toolbox->Name;
        else if (known)
            name = known->Name;
        else if (std::optional<std::string> npdmName = NpdmReader::ReadModuleName(entry.path()))
            name = *npdmName;

        bool requiresReboot = (known && known->RequiresReboot) || (toolbox && toolbox->RequiresReboot);
        ModuleWarning warning = known ? known->Warning : ModuleWarning::None;
        return ModuleInfo(*programId, name, entry.path(), requiresReboot, warning);
    }

    std::optional<std::vector<ModuleInfo>> ModuleScanner::Scan(const std::filesystem::path& contentsDirectory)
    {
        std::error_code error;
        std::filesystem::directory_iterator iterator(contentsDirectory, error);
        if (error)
            return std::nullopt;
        std::vector<ModuleInfo> modules;
        for (const std::filesystem::directory_entry& entry : iterator)
        {
            std::optional<ModuleInfo> module = Describe(entry);
            if (module)
                modules.push_back(*module);
        }
        std::sort(modules.begin(), modules.end(), [](const ModuleInfo& a, const ModuleInfo& b)
        {
            return ToLower(a.Name()) < ToLower(b.Name());
        });
        return modules;
    }
}
