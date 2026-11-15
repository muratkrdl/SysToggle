#pragma once

#include <optional>
#include <string>
#include <vector>

#include <borealis.hpp>

#include "core/ModuleInfo.hpp"
#include "core/Profile.hpp"

namespace SysToggle::Ui
{
    class ProfilesTab : public brls::Box
    {
    public:
        ProfilesTab();

        static brls::View* Create();

    private:
        void Rebuild();
        brls::DetailCell* CreateProfileCell(const Core::Profile& profile);
        void ShowStatus(const std::string& text);
        std::optional<std::vector<Core::ModuleInfo>> ScanModules();

        void AskNameForNewProfile();
        void SaveCurrentAs(const std::string& name);
        void ConfirmApply(const std::string& name);
        void Apply(const std::string& name, bool reboot);
        void ConfirmOverwrite(const std::string& name);
        void ConfirmDelete(const std::string& name);
        bool Persist();

        BRLS_BIND(brls::DetailCell, m_SaveNewCell, "save_new");
        BRLS_BIND(brls::Label, m_StatusLabel, "status");
        BRLS_BIND(brls::Box, m_ListBox, "list");

        Core::ProfileStore m_Store;
    };
}
