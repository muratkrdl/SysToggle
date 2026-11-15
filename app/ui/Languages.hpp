#pragma once

#include <array>
#include <string>

#include <borealis.hpp>

namespace SysToggle::Ui
{
    struct LanguageOption
    {
        const char* Code;
        const char* NativeName;
        const char* Locale;
    };

    class Languages
    {
    public:
        static constexpr const char* k_AutoCode = "auto";

        static constexpr std::array<LanguageOption, 14> k_Options{{
            { k_AutoCode, nullptr, "auto" },
            { "en", "English", "en-US" },
            { "tr", "Türkçe", "tr" },
            { "ja", "日本語", "ja" },
            { "fr", "Français", "fr" },
            { "de", "Deutsch", "de" },
            { "it", "Italiano", "it" },
            { "es", "Español", "es" },
            { "nl", "Nederlands", "nl" },
            { "pt", "Português", "pt" },
            { "ru", "Русский", "ru" },
            { "ko", "한국어", "ko" },
            { "zh-Hans", "简体中文", "zh-Hans" },
            { "zh-Hant", "繁體中文", "zh-Hant" },
        }};

        static int IndexOf(const std::string& code)
        {
            for (std::size_t i = 0; i < k_Options.size(); ++i)
            {
                if (code == k_Options[i].Code)
                    return static_cast<int>(i);
            }
            return 0;
        }

        static const char* LocaleFor(const std::string& code)
        {
            return k_Options[IndexOf(code)].Locale;
        }

        static std::string DisplayName(const LanguageOption& option)
        {
            if (option.NativeName == nullptr)
                return brls::getStr("app/settings/language_auto");
            return option.NativeName;
        }
    };
}
