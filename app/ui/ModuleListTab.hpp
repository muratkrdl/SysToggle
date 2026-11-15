#pragma once

#include <string>
#include <vector>

#include <borealis.hpp>

#include "core/ProcessControl.hpp"
#include "ui/ModuleCell.hpp"

namespace SysToggle::Ui
{
    class ModuleListTab : public brls::Box
    {
    public:
        ModuleListTab();

        static brls::View* Create();

    private:
        void Populate();
        void ShowStatus(const std::string& text);
        void RefreshMemory();
        void RefreshRunningStates();
        void ShowRebootPrompt();
        bool OnRebootClicked();

        BRLS_BIND(brls::DetailCell, m_MemoryCell, "memory");
        BRLS_BIND(brls::DetailCell, m_RebootCell, "reboot");
        BRLS_BIND(brls::Label, m_StatusLabel, "status");
        BRLS_BIND(brls::Box, m_ListBox, "list");

        Core::ProcessControl m_ProcessControl;
        std::vector<ModuleCell*> m_Cells;
        brls::RepeatingTimer m_RefreshTimer;
    };
}
