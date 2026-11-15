#include <cstdlib>

#include <borealis.hpp>

#include "core/AppSettings.hpp"
#include "core/Paths.hpp"
#include "ui/Languages.hpp"
#include "ui/MainActivity.hpp"
#include "ui/ModuleListTab.hpp"
#include "ui/ProfilesTab.hpp"
#include "ui/SettingsTab.hpp"

using namespace brls::literals;

namespace
{
    void ApplySavedLanguage()
    {
        SysToggle::Core::AppSettings settings(SysToggle::Core::Paths::k_SettingsFile);
        if (!settings.Load())
            brls::Logger::info("No saved settings, using automatic language");
        brls::Platform::APP_LOCALE_DEFAULT = SysToggle::Ui::Languages::LocaleFor(settings.Language());
    }
}

int main()
{
    ApplySavedLanguage();

    if (!brls::Application::init())
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::Application::createWindow("app/title"_i18n);
    brls::Application::getPlatform()->setThemeVariant(brls::ThemeVariant::DARK);
    brls::Application::setGlobalQuit(true);

    brls::Application::registerXMLView("ModuleListTab", SysToggle::Ui::ModuleListTab::Create);
    brls::Application::registerXMLView("ProfilesTab", SysToggle::Ui::ProfilesTab::Create);
    brls::Application::registerXMLView("SettingsTab", SysToggle::Ui::SettingsTab::Create);

    brls::Application::pushActivity(new SysToggle::Ui::MainActivity());

    while (brls::Application::mainLoop())
        ;

    return EXIT_SUCCESS;
}
