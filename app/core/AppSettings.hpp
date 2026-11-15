#pragma once

#include <filesystem>
#include <string>

namespace SysToggle::Core
{
    class AppSettings
    {
    public:
        explicit AppSettings(std::filesystem::path file);

        const std::string& Language() const { return m_Language; }
        void SetLanguage(const std::string& language) { m_Language = language; }

        bool Load();
        bool Save() const;

    private:
        std::filesystem::path m_File;
        std::string m_Language;
    };
}
