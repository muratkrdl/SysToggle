#include "core/AppSettings.hpp"

#include <fstream>
#include <system_error>
#include <utility>

#include <borealis/extern/nlohmann/json.hpp>

namespace SysToggle::Core
{
    namespace
    {
        constexpr const char* k_DefaultLanguage = "auto";
        constexpr const char* k_LanguageKey = "language";
        constexpr int k_JsonIndent = 2;
    }

    AppSettings::AppSettings(std::filesystem::path file)
        : m_File(std::move(file))
        , m_Language(k_DefaultLanguage)
    {
    }

    bool AppSettings::Load()
    {
        std::ifstream stream(m_File);
        if (!stream)
            return false;
        nlohmann::json json = nlohmann::json::parse(stream, nullptr, false);
        if (json.is_discarded() || !json.is_object())
            return false;
        m_Language = json.value(k_LanguageKey, std::string(k_DefaultLanguage));
        return true;
    }

    bool AppSettings::Save() const
    {
        std::error_code error;
        std::filesystem::create_directories(m_File.parent_path(), error);
        if (error)
            return false;
        std::ofstream stream(m_File, std::ios::trunc);
        if (!stream)
            return false;
        nlohmann::json json{ { k_LanguageKey, m_Language } };
        stream << json.dump(k_JsonIndent);
        return static_cast<bool>(stream);
    }
}
