#include "ui/ModuleListTab.hpp"

#include <optional>

#include "core/ModuleScanner.hpp"
#include "core/Paths.hpp"
#include "core/SystemInfo.hpp"

using namespace brls::literals;

namespace SysToggle::Ui
{
    namespace
    {
        constexpr brls::Time k_RefreshPeriodMs = 1000;
        constexpr std::uint64_t k_BytesPerMegabyte = 1024 * 1024;
    }

    ModuleListTab::ModuleListTab()
    {
        inflateFromXMLRes("xml/tabs/modules.xml");

        m_RebootCell->setDetailText("app/reboot/detail"_i18n);
        m_RebootCell->registerClickAction([this](brls::View*) { return OnRebootClicked(); });

        Populate();
        RefreshMemory();

        m_RefreshTimer.setCallback([this]
        {
            RefreshMemory();
            RefreshRunningStates();
        });
        m_RefreshTimer.start(k_RefreshPeriodMs);
    }

    brls::View* ModuleListTab::Create()
    {
        return new ModuleListTab();
    }

    void ModuleListTab::Populate()
    {
        m_ListBox->clearViews();
        m_Cells.clear();

        std::optional<std::vector<Core::ModuleInfo>> modules = Core::ModuleScanner::Scan(Core::Paths::k_ContentsDirectory);
        if (!modules)
        {
            brls::Logger::error("Cannot read {}", Core::Paths::k_ContentsDirectory);
            ShowStatus("app/list/error"_i18n);
            return;
        }
        if (modules->empty())
        {
            ShowStatus("app/list/empty"_i18n);
            return;
        }

        m_Cells.reserve(modules->size());
        for (Core::ModuleInfo& module : *modules)
        {
            auto* cell = new ModuleCell(std::move(module), m_ProcessControl, [this] { ShowRebootPrompt(); });
            m_ListBox->addView(cell);
            m_Cells.push_back(cell);
        }
    }

    void ModuleListTab::ShowStatus(const std::string& text)
    {
        m_StatusLabel->setText(text);
        m_StatusLabel->setVisibility(brls::Visibility::VISIBLE);
    }

    void ModuleListTab::RefreshMemory()
    {
        std::optional<Core::MemoryUsage> usage = Core::SystemInfo::QuerySystemPool();
        if (!usage)
        {
            m_MemoryCell->setDetailText("app/memory/unavailable"_i18n);
            return;
        }
        m_MemoryCell->setDetailText(brls::getStr("app/memory/value",
            usage->FreeBytes() / k_BytesPerMegabyte, usage->TotalBytes / k_BytesPerMegabyte));
    }

    void ModuleListTab::RefreshRunningStates()
    {
        for (ModuleCell* cell : m_Cells)
            cell->Refresh();
    }

    void ModuleListTab::ShowRebootPrompt()
    {
        m_RebootCell->setVisibility(brls::Visibility::VISIBLE);
    }

    bool ModuleListTab::OnRebootClicked()
    {
        if (Core::SystemInfo::Reboot())
            return true;
        brls::Logger::error("Reboot request failed");
        brls::Application::notify("app/reboot/failed"_i18n);
        return true;
    }
}
