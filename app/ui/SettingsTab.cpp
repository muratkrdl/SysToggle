#include "ui/SettingsTab.hpp"

#include <string>
#include <vector>

#include "core/Paths.hpp"
#include "ui/Languages.hpp"

using namespace brls::literals;

namespace SysToggle::Ui
{
    SettingsTab::SettingsTab()
        : m_Settings(Core::Paths::k_SettingsFile)
    {
        inflateFromXMLRes("xml/tabs/settings.xml");

        m_Settings.Load();

        std::vector<std::string> labels;
        labels.reserve(Languages::k_Options.size());
        for (const LanguageOption& option : Languages::k_Options)
            labels.push_back(Languages::DisplayName(option));

        m_LanguageCell->init("app/settings/language"_i18n, labels,
            Languages::IndexOf(m_Settings.Language()),
            [this](int index) { OnLanguageSelected(index); });

        m_VersionCell->setDetailText(APP_VERSION_TEXT);
        m_AuthorCell->setDetailText(APP_AUTHOR_TEXT);
        m_SourceCell->setDetailText(APP_SOURCE_URL);
    }

    brls::View* SettingsTab::Create()
    {
        return new SettingsTab();
    }

    void SettingsTab::OnLanguageSelected(int index)
    {
        m_Settings.SetLanguage(Languages::k_Options[index].Code);
        if (!m_Settings.Save())
        {
            brls::Logger::error("Cannot save {}", Core::Paths::k_SettingsFile);
            brls::Application::notify("app/settings/save_failed"_i18n);
            return;
        }
        brls::Application::notify("app/settings/restart"_i18n);
    }
}
