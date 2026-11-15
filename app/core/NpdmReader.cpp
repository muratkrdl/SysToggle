#include "core/NpdmReader.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

namespace SysToggle::Core
{
    namespace
    {
        constexpr const char* k_ExefsFile = "exefs.nsp";
        constexpr const char* k_ExefsDirectory = "exefs";
        constexpr const char* k_NpdmFile = "main.npdm";

        constexpr std::uint32_t k_Pfs0Magic = 0x30534650;
        constexpr std::size_t k_Pfs0HeaderSize = 0x10;
        constexpr std::size_t k_Pfs0EntrySize = 0x18;
        constexpr std::uint32_t k_MaxPfs0Entries = 64;
        constexpr std::uint32_t k_MaxStringTableSize = 0x1000;
        constexpr std::uint64_t k_MaxNpdmSize = 0x10000;

        constexpr std::uint32_t k_MetaMagic = 0x4154454D;
        constexpr std::size_t k_MetaNameOffset = 0x20;
        constexpr std::size_t k_MetaNameLength = 0x10;

        template <typename T>
        bool ReadValue(std::istream& stream, T& value)
        {
            return static_cast<bool>(stream.read(reinterpret_cast<char*>(&value), sizeof(T)));
        }

        bool IsPrintableName(const std::string& name)
        {
            return !name.empty() && std::all_of(name.begin(), name.end(),
                [](unsigned char c) { return c >= 0x20 && c < 0x7F; });
        }
    }

    std::optional<std::string> NpdmReader::ReadModuleName(const std::filesystem::path& moduleDirectory)
    {
        std::optional<std::vector<std::uint8_t>> npdm = ExtractFromPfs0(moduleDirectory / k_ExefsFile, k_NpdmFile);
        if (!npdm)
            npdm = ReadFile(moduleDirectory / k_ExefsDirectory / k_NpdmFile);
        if (!npdm)
            return std::nullopt;
        return NameFromNpdm(*npdm);
    }

    std::optional<std::string> NpdmReader::NameFromNpdm(const std::vector<std::uint8_t>& npdm)
    {
        if (npdm.size() < k_MetaNameOffset + k_MetaNameLength)
            return std::nullopt;
        std::uint32_t magic = 0;
        std::memcpy(&magic, npdm.data(), sizeof(magic));
        if (magic != k_MetaMagic)
            return std::nullopt;
        const char* nameStart = reinterpret_cast<const char*>(npdm.data() + k_MetaNameOffset);
        std::string name(nameStart, strnlen(nameStart, k_MetaNameLength));
        if (!IsPrintableName(name))
            return std::nullopt;
        return name;
    }

    std::optional<std::vector<std::uint8_t>> NpdmReader::ExtractFromPfs0(const std::filesystem::path& nspFile,
        const std::string& entryName)
    {
        std::ifstream stream(nspFile, std::ios::binary);
        if (!stream)
            return std::nullopt;

        std::uint32_t magic = 0;
        std::uint32_t entryCount = 0;
        std::uint32_t stringTableSize = 0;
        std::uint32_t reserved = 0;
        bool headerRead = ReadValue(stream, magic) && ReadValue(stream, entryCount)
            && ReadValue(stream, stringTableSize) && ReadValue(stream, reserved);
        if (!headerRead || magic != k_Pfs0Magic || entryCount > k_MaxPfs0Entries
            || stringTableSize > k_MaxStringTableSize)
            return std::nullopt;

        std::vector<char> entries(entryCount * k_Pfs0EntrySize);
        std::vector<char> strings(stringTableSize + 1, '\0');
        if (!stream.read(entries.data(), entries.size()) || !stream.read(strings.data(), stringTableSize))
            return std::nullopt;

        std::uint64_t dataStart = k_Pfs0HeaderSize + entries.size() + stringTableSize;
        for (std::uint32_t i = 0; i < entryCount; ++i)
        {
            std::uint64_t offset = 0;
            std::uint64_t size = 0;
            std::uint32_t nameOffset = 0;
            const char* entry = entries.data() + i * k_Pfs0EntrySize;
            std::memcpy(&offset, entry, sizeof(offset));
            std::memcpy(&size, entry + 0x8, sizeof(size));
            std::memcpy(&nameOffset, entry + 0x10, sizeof(nameOffset));
            if (nameOffset >= stringTableSize || entryName != strings.data() + nameOffset)
                continue;
            if (size > k_MaxNpdmSize)
                return std::nullopt;

            std::vector<std::uint8_t> data(size);
            stream.seekg(static_cast<std::streamoff>(dataStart + offset));
            if (!stream.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size)))
                return std::nullopt;
            return data;
        }
        return std::nullopt;
    }

    std::optional<std::vector<std::uint8_t>> NpdmReader::ReadFile(const std::filesystem::path& file)
    {
        std::ifstream stream(file, std::ios::binary);
        if (!stream)
            return std::nullopt;
        std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        if (data.size() > k_MaxNpdmSize)
            return std::nullopt;
        return data;
    }
}
