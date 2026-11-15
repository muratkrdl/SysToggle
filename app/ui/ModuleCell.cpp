#include "ui/ModuleCell.hpp"

#include <utility>

#include "core/BootFlag.hpp"

using namespace brls::literals;

namespace SysToggle::Ui
{
    namespace
    {
        constexpr const char* k_StateSeparator = "  ·  ";
    }

    ModuleCell::ModuleCell(Core::ModuleInfo module, const Core::ProcessControl& processControl,
        std::function<void()> bootChanged)
        : m_Module(std::move(module))
        , m_ProcessControl(processControl)
        , m_BootChanged(std::move(bootChanged))
        , m_ProcessState(Core::ProcessState::Unknown)
    {
        init(m_Module.Name(), Core::BootFlag::IsEnabled(m_Module.Directory()),
            [this](bool enabled) { OnBootToggled(enabled); });

        registerAction("app/action/start"_i18n, brls::BUTTON_X,
            [this](brls::View*) { return OnLiveAction(); }, false, false, brls::SOUND_CLICK);
        registerAction("app/action/details"_i18n, brls::BUTTON_Y,
            [this](brls::View*) { return OnDetailsAction(); }, false, false, brls::SOUND_CLICK);

        RefreshRunningState();
    }

    void ModuleCell::Refresh()
    {
        bool bootEnabled = Core::BootFlag::IsEnabled(m_Module.Directory());
        if (bootEnabled != isOn())
            setOn(bootEnabled, false);
        RefreshRunningState();
    }

    void ModuleCell::RefreshRunningState()
    {
        Core::ProcessState state = m_ProcessControl.QueryState(m_Module.ProgramId());
        if (state == m_ProcessState)
            return;
        m_ProcessState = state;
        UpdateTitle();
        UpdateLiveHint();
    }

    void ModuleCell::UpdateTitle()
    {
        if (m_ProcessState == Core::ProcessState::Unknown)
        {
            title->setText(m_Module.Name());
            return;
        }
        bool running = m_ProcessState == Core::ProcessState::Running;
        std::string state = running ? "app/state/running"_i18n : "app/state/stopped"_i18n;
        title->setText(m_Module.Name() + k_StateSeparator + state);
    }

    void ModuleCell::UpdateLiveHint()
    {
        bool running = m_ProcessState == Core::ProcessState::Running;
        updateActionHint(brls::BUTTON_X, running ? "app/action/stop"_i18n : "app/action/start"_i18n);
    }

    void ModuleCell::OnBootToggled(bool enabled)
    {
        bool needsConfirmation = !enabled && m_Module.Warning() != Core::ModuleWarning::None;
        if (!needsConfirmation)
        {
            ApplyBootFlag(enabled);
            return;
        }
        setOn(true, false);
        ConfirmDisable();
    }

    void ModuleCell::ApplyBootFlag(bool enabled)
    {
        bool applied = Core::BootFlag::Set(m_Module.Directory(), enabled);
        setOn(Core::BootFlag::IsEnabled(m_Module.Directory()), false);
        if (!applied)
        {
            brls::Logger::error("boot2.flag change failed for {}", m_Module.ProgramIdText());
            brls::Application::notify(brls::getStr("app/notify/flag_failed", m_Module.Name()));
            return;
        }
        m_BootChanged();
    }

    void ModuleCell::ConfirmDisable()
    {
        auto* dialog = new brls::Dialog(WarningText());
        dialog->addButton("hints/cancel"_i18n, [] {});
        dialog->addButton("app/action/turn_off"_i18n, [this] { ApplyBootFlag(false); });
        dialog->open();
    }

    bool ModuleCell::OnLiveAction()
    {
        if (!m_ProcessControl.IsAvailable())
        {
            brls::Application::notify("app/notify/live_unavailable"_i18n);
            return true;
        }
        if (m_Module.RequiresReboot())
        {
            brls::Application::notify(brls::getStr("app/notify/requires_reboot", m_Module.Name()));
            return true;
        }

        bool stopping = m_ProcessState == Core::ProcessState::Running;
        bool succeeded = stopping
            ? m_ProcessControl.Terminate(m_Module.ProgramId())
            : m_ProcessControl.Launch(m_Module.ProgramId());
        if (succeeded)
            brls::Application::notify(brls::getStr(stopping ? "app/notify/stopped" : "app/notify/started", m_Module.Name()));
        else
        {
            brls::Logger::error("live control failed for {}", m_Module.ProgramIdText());
            brls::Application::notify(brls::getStr("app/notify/live_failed", m_Module.Name()));
        }
        RefreshRunningState();
        return true;
    }

    bool ModuleCell::OnDetailsAction()
    {
        auto* dialog = new brls::Dialog(DetailsText());
        dialog->addButton("hints/ok"_i18n, [] {});
        dialog->open();
        return true;
    }

    std::string ModuleCell::WarningText() const
    {
        bool isLoader = m_Module.Warning() == Core::ModuleWarning::OverlayLoader;
        return brls::getStr(isLoader ? "app/warning/overlay_loader" : "app/warning/signature_patches", m_Module.Name());
    }

    std::string ModuleCell::DetailsText() const
    {
        bool bootEnabled = Core::BootFlag::IsEnabled(m_Module.Directory());
        std::string boot = bootEnabled ? "app/details/yes"_i18n : "app/details/no"_i18n;
        return m_Module.Name() + "\n\n"
            + brls::getStr("app/details/program_id", m_Module.ProgramIdText()) + "\n"
            + brls::getStr("app/details/path", m_Module.Directory().string()) + "\n"
            + brls::getStr("app/details/boot", boot) + "\n\n"
            + (m_Module.RequiresReboot() ? "app/details/live_reboot"_i18n : "app/details/live_ok"_i18n);
    }
}
