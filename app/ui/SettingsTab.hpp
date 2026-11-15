#pragma once

#include <borealis.hpp>

#include "core/AppSettings.hpp"

namespace SysToggle::Ui
{
    class SettingsTab : public brls::Box
    {
    public:
        SettingsTab();

        static brls::View* Create();

    private:
        void OnLanguageSelected(int index);

        BRLS_BIND(brls::SelectorCell, m_LanguageCell, "language");
        BRLS_BIND(brls::DetailCell, m_VersionCell, "version");
        BRLS_BIND(brls::DetailCell, m_AuthorCell, "author");
        BRLS_BIND(brls::DetailCell, m_SourceCell, "source");

        Core::AppSettings m_Settings;
    };
}
