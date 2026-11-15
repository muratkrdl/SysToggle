#include "ui/ProfilesTab.hpp"

#include <utility>

#include "core/ModuleScanner.hpp"
#include "core/Paths.hpp"
#include "core/SystemInfo.hpp"

using namespace brls::literals;

namespace SysToggle::Ui
{
    namespace
    {
        constexpr int k_MaxProfileNameLength = 24;
    }

    ProfilesTab::ProfilesTab()
        : m_Store(Core::Paths::k_ProfilesFile)
    {
        inflateFromXMLRes("xml/tabs/profiles.xml");

        m_SaveNewCell->registerClickAction([this](brls::View*)
        {
            AskNameForNewProfile();
            return true;
        });

        if (!m_Store.Load())
            brls::Logger::info("No profiles file yet: {}", Core::Paths::k_ProfilesFile);
        Rebuild();
    }

    brls::View* ProfilesTab::Create()
    {
        return new ProfilesTab();
    }

    void ProfilesTab::Rebuild()
    {
        if (!m_ListBox->getChildren().empty())
            brls::Application::giveFocus(m_SaveNewCell.getView());
        m_ListBox->clearViews();
        if (m_Store.Profiles().empty())
        {
            ShowStatus("app/profiles/empty"_i18n);
            return;
        }
        m_StatusLabel->setVisibility(brls::Visibility::GONE);
        for (const Core::Profile& profile : m_Store.Profiles())
            m_ListBox->addView(CreateProfileCell(profile));
    }

    brls::DetailCell* ProfilesTab::CreateProfileCell(const Core::Profile& profile)
    {
        auto* cell = new brls::DetailCell();
        std::size_t enabled = profile.EnabledCount();
        std::size_t disabled = profile.BootStates().size() - enabled;
        cell->setText(profile.Name());
        cell->setDetailText(brls::getStr("app/profiles/summary", enabled, disabled));

        std::string name = profile.Name();
        cell->registerClickAction([this, name](brls::View*)
        {
            ConfirmApply(name);
            return true;
        });
        cell->registerAction("app/profiles/overwrite"_i18n, brls::BUTTON_X, [this, name](brls::View*)
        {
            ConfirmOverwrite(name);
            return true;
        }, false, false, brls::SOUND_CLICK);
        cell->registerAction("app/profiles/delete"_i18n, brls::BUTTON_Y, [this, name](brls::View*)
        {
            ConfirmDelete(name);
            return true;
        }, false, false, brls::SOUND_CLICK);
        return cell;
    }

    void ProfilesTab::ShowStatus(const std::string& text)
    {
        m_StatusLabel->setText(text);
        m_StatusLabel->setVisibility(brls::Visibility::VISIBLE);
    }

    std::optional<std::vector<Core::ModuleInfo>> ProfilesTab::ScanModules()
    {
        std::optional<std::vector<Core::ModuleInfo>> modules = Core::ModuleScanner::Scan(Core::Paths::k_ContentsDirectory);
        if (!modules)
        {
            brls::Logger::error("Cannot read {}", Core::Paths::k_ContentsDirectory);
            brls::Application::notify("app/list/error"_i18n);
        }
        return modules;
    }

    void ProfilesTab::AskNameForNewProfile()
    {
        bool opened = brls::Application::getImeManager()->openForText(
            [this](std::string name)
            {
                if (name.empty())
                    return;
                if (m_Store.IndexOf(name))
                    ConfirmOverwrite(name);
                else
                    SaveCurrentAs(name);
            },
            "app/profiles/name_header"_i18n, "app/profiles/name_hint"_i18n, k_MaxProfileNameLength);
        if (!opened)
        {
            brls::Logger::error("Software keyboard could not be opened");
            brls::Application::notify("app/profiles/keyboard_failed"_i18n);
        }
    }

    void ProfilesTab::SaveCurrentAs(const std::string& name)
    {
        std::optional<std::vector<Core::ModuleInfo>> modules = ScanModules();
        if (!modules)
            return;
        m_Store.Upsert(Core::Profile::Capture(name, *modules));
        if (!Persist())
            return;
        Rebuild();
        brls::Application::notify(brls::getStr("app/profiles/saved", name));
    }

    void ProfilesTab::ConfirmApply(const std::string& name)
    {
        auto* dialog = new brls::Dialog(brls::getStr("app/profiles/apply_question", name));
        dialog->addButton("hints/cancel"_i18n, [] {});
        dialog->addButton("app/profiles/apply_only"_i18n, [this, name] { Apply(name, false); });
        dialog->addButton("app/profiles/apply_reboot"_i18n, [this, name] { Apply(name, true); });
        dialog->open();
    }

    void ProfilesTab::Apply(const std::string& name, bool reboot)
    {
        std::optional<std::size_t> index = m_Store.IndexOf(name);
        std::optional<std::vector<Core::ModuleInfo>> modules = ScanModules();
        if (!index || !modules)
            return;

        Core::ApplyResult result = Core::ProfileApplier::Apply(m_Store.Profiles()[*index], *modules);
        if (!result.Succeeded())
        {
            brls::Logger::error("Profile {} failed for {} module(s)", name, result.Failed);
            brls::Application::notify(brls::getStr("app/profiles/apply_failed", result.Failed));
            return;
        }
        brls::Application::notify(brls::getStr("app/profiles/applied", name, result.Changed, result.Missing));
        if (!reboot)
            return;
        if (!Core::SystemInfo::Reboot())
        {
            brls::Logger::error("Reboot request failed");
            brls::Application::notify("app/reboot/failed"_i18n);
        }
    }

    void ProfilesTab::ConfirmOverwrite(const std::string& name)
    {
        auto* dialog = new brls::Dialog(brls::getStr("app/profiles/overwrite_question", name));
        dialog->addButton("hints/cancel"_i18n, [] {});
        dialog->addButton("app/profiles/overwrite"_i18n, [this, name] { SaveCurrentAs(name); });
        dialog->open();
    }

    void ProfilesTab::ConfirmDelete(const std::string& name)
    {
        auto* dialog = new brls::Dialog(brls::getStr("app/profiles/delete_question", name));
        dialog->addButton("hints/cancel"_i18n, [] {});
        dialog->addButton("app/profiles/delete"_i18n, [this, name]
        {
            if (!m_Store.Remove(name))
            {
                brls::Logger::error("Profile {} was not found for deletion", name);
                return;
            }
            if (Persist())
                Rebuild();
        });
        dialog->open();
    }

    bool ProfilesTab::Persist()
    {
        if (m_Store.Save())
            return true;
        brls::Logger::error("Cannot save {}", Core::Paths::k_ProfilesFile);
        brls::Application::notify("app/profiles/save_failed"_i18n);
        return false;
    }
}
