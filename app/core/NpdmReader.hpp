#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace SysToggle::Core
{
    class NpdmReader
    {
    public:
        static std::optional<std::string> ReadModuleName(const std::filesystem::path& moduleDirectory);
        static std::optional<std::string> NameFromNpdm(const std::vector<std::uint8_t>& npdm);
        static std::optional<std::vector<std::uint8_t>> ExtractFromPfs0(const std::filesystem::path& nspFile,
            const std::string& entryName);

    private:
        static std::optional<std::vector<std::uint8_t>> ReadFile(const std::filesystem::path& file);
    };
}
