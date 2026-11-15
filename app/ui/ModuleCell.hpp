#pragma once

#include <functional>
#include <string>

#include <borealis.hpp>

#include "core/ModuleInfo.hpp"
#include "core/ProcessControl.hpp"

namespace SysToggle::Ui
{
    class ModuleCell : public brls::BooleanCell
    {
    public:
        ModuleCell(Core::ModuleInfo module, const Core::ProcessControl& processControl,
            std::function<void()> bootChanged);

        void Refresh();

    private:
        void RefreshRunningState();
        void OnBootToggled(bool enabled);
        void ApplyBootFlag(bool enabled);
        void ConfirmDisable();
        bool OnLiveAction();
        bool OnDetailsAction();
        void UpdateTitle();
        void UpdateLiveHint();
        std::string WarningText() const;
        std::string DetailsText() const;

        Core::ModuleInfo m_Module;
        const Core::ProcessControl& m_ProcessControl;
        std::function<void()> m_BootChanged;
        Core::ProcessState m_ProcessState;
    };
}
