#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "core/AppSettings.hpp"
#include "core/BootFlag.hpp"
#include "core/ModuleScanner.hpp"
#include "core/NpdmReader.hpp"
#include "core/Profile.hpp"

using namespace SysToggle::Core;
namespace fs = std::filesystem;

namespace
{
    int s_Failures = 0;

    void Expect(bool condition, const std::string& message)
    {
        if (condition)
            return;
        ++s_Failures;
        std::printf("  FAIL: %s\n", message.c_str());
    }

    void Touch(const fs::path& file, const std::string& content = "")
    {
        fs::create_directories(file.parent_path());
        std::ofstream(file, std::ios::binary) << content;
    }

    fs::path MakeFixture(const fs::path& root)
    {
        fs::remove_all(root);
        fs::path contents = root / "atmosphere" / "contents";

        Touch(contents / "00FF0000636C6BFF" / "exefs.nsp");
        Touch(contents / "00FF0000636C6BFF" / "flags" / "boot2.flag");

        Touch(contents / "420000000007E51A" / "exefs.nsp");
        Touch(contents / "420000000007E51A" / "toolbox.json",
            R"({"name":"nx-ovlloader+","tid":"420000000007E51A","requires_reboot":true})");
        fs::create_directories(contents / "420000000007E51A" / "flags");

        Touch(contents / "4200000000ABCDEF" / "exefs" / "main.npdm");
        Touch(contents / "4200000000ABCDEF" / "toolbox.json", R"({"name":"Custom Mod","requires_reboot":false})");

        Touch(contents / "0100F2C0115B6000" / "romfs" / "Font" / "font.bfarc.zs");
        Touch(contents / "01004AB00A260000" / "cheats" / "0123.txt");
        Touch(contents / "0100ABCD00000000" / "exefs" / "main.npdm");

        Touch(contents / "1234567890ABCDEF" / "exefs.nsp");
        Touch(contents / "1234567890ABCDEF" / "toolbox.json", "{ broken json");

        Touch(contents / "not-a-program-id" / "exefs.nsp");
        return contents;
    }

    const ModuleInfo* FindModule(const std::vector<ModuleInfo>& modules, std::uint64_t programId)
    {
        for (const ModuleInfo& module : modules)
        {
            if (module.ProgramId() == programId)
                return &module;
        }
        return nullptr;
    }

    void TestParseProgramId()
    {
        Expect(ModuleScanner::ParseProgramId("00FF0000636C6BFF") == 0x00FF0000636C6BFFull, "parses upper hex");
        Expect(ModuleScanner::ParseProgramId("010000000000bd00") == 0x010000000000BD00ull, "parses lower hex");
        Expect(!ModuleScanner::ParseProgramId("01008C~1").has_value(), "rejects short 8.3 name");
        Expect(!ModuleScanner::ParseProgramId("ZZFF0000636C6BFF").has_value(), "rejects non hex");
    }

    void TestScan(const fs::path& contents)
    {
        std::optional<std::vector<ModuleInfo>> scanned = ModuleScanner::Scan(contents);
        Expect(scanned.has_value(), "scan succeeds");
        if (!scanned)
            return;
        const std::vector<ModuleInfo>& modules = *scanned;

        Expect(modules.size() == 4, "finds exactly four sysmodules, found " + std::to_string(modules.size()));

        const ModuleInfo* sysClk = FindModule(modules, 0x00FF0000636C6BFFull);
        Expect(sysClk && sysClk->Name() == "sys-clk", "sys-clk named from known table");
        Expect(sysClk && !sysClk->RequiresReboot(), "sys-clk is live controllable");

        const ModuleInfo* loader = FindModule(modules, 0x420000000007E51Aull);
        Expect(loader && loader->Name() == "nx-ovlloader+", "toolbox name wins over known table");
        Expect(loader && loader->RequiresReboot(), "nx-ovlloader requires reboot");
        Expect(loader && loader->Warning() == ModuleWarning::OverlayLoader, "nx-ovlloader carries warning");

        const ModuleInfo* custom = FindModule(modules, 0x4200000000ABCDEFull);
        Expect(custom && custom->Name() == "Custom Mod", "exefs directory sysmodule with toolbox detected");

        const ModuleInfo* broken = FindModule(modules, 0x1234567890ABCDEFull);
        Expect(broken && broken->Name() == "1234567890ABCDEF", "broken toolbox falls back to program id");

        Expect(!FindModule(modules, 0x0100F2C0115B6000ull), "romfs game mod ignored");
        Expect(!FindModule(modules, 0x01004AB00A260000ull), "cheats folder ignored");
        Expect(!FindModule(modules, 0x0100ABCD00000000ull), "bare game exefs mod ignored");

        Expect(modules.front().Name() == "1234567890ABCDEF", "sorted by name");
    }

    void TestScanMissingDirectory(const fs::path& root)
    {
        Expect(!ModuleScanner::Scan(root / "does-not-exist").has_value(), "missing contents reports failure");
    }

    void TestBootFlag(const fs::path& contents)
    {
        fs::path sysClk = contents / "00FF0000636C6BFF";
        Expect(BootFlag::IsEnabled(sysClk), "sys-clk starts enabled");
        Expect(BootFlag::Disable(sysClk), "disable succeeds");
        Expect(!BootFlag::IsEnabled(sysClk), "disabled after disable");
        Expect(BootFlag::Enable(sysClk), "enable succeeds");
        Expect(BootFlag::IsEnabled(sysClk), "enabled after enable");

        fs::path custom = contents / "4200000000ABCDEF";
        Expect(!fs::exists(custom / "flags"), "custom has no flags directory");
        Expect(BootFlag::Set(custom, true), "enable creates flags directory");
        Expect(BootFlag::IsEnabled(custom), "custom enabled");
        Expect(fs::file_size(custom / "flags" / "boot2.flag") == 0, "flag file is empty");
        Expect(BootFlag::Set(custom, false), "disable custom");
        Expect(BootFlag::Disable(custom), "disabling twice still succeeds");
    }

    void TestSettings(const fs::path& root)
    {
        fs::path file = root / "config" / "SysToggle" / "settings.json";
        AppSettings missing(file);
        Expect(!missing.Load(), "missing settings file reports failure");
        Expect(missing.Language() == "auto", "default language is auto");

        missing.SetLanguage("tr");
        Expect(missing.Save(), "save creates directories");

        AppSettings loaded(file);
        Expect(loaded.Load() && loaded.Language() == "tr", "language round trips");

        Touch(file, "not json");
        AppSettings corrupt(file);
        Expect(!corrupt.Load() && corrupt.Language() == "auto", "corrupt settings keep default");
    }

    std::vector<std::uint8_t> MakeNpdm(const std::string& name)
    {
        std::vector<std::uint8_t> npdm(0x80, 0);
        const char magic[] = { 'M', 'E', 'T', 'A' };
        std::copy(std::begin(magic), std::end(magic), npdm.begin());
        std::copy(name.begin(), name.end(), npdm.begin() + 0x20);
        return npdm;
    }

    void TestNpdm()
    {
        Expect(NpdmReader::NameFromNpdm(MakeNpdm("MyModule")) == std::string("MyModule"), "reads META name");
        Expect(NpdmReader::NameFromNpdm(MakeNpdm("ExactlySixteen!!")) == std::string("ExactlySixteen!!"),
            "reads 16 byte name without terminator");
        Expect(!NpdmReader::NameFromNpdm(MakeNpdm("")).has_value(), "empty name rejected");
        std::vector<std::uint8_t> wrongMagic = MakeNpdm("Name");
        wrongMagic[0] = 'X';
        Expect(!NpdmReader::NameFromNpdm(wrongMagic).has_value(), "wrong magic rejected");
        Expect(!NpdmReader::NameFromNpdm(std::vector<std::uint8_t>(8, 0)).has_value(), "short buffer rejected");
    }

    void TestRealNsp(const fs::path& contents, const fs::path& sampleNsp)
    {
        if (sampleNsp.empty() || !fs::exists(sampleNsp))
        {
            std::printf("  SKIP: no sample nsp given\n");
            return;
        }
        fs::path module = contents / "0100000000000123";
        fs::create_directories(module);
        fs::copy_file(sampleNsp, module / "exefs.nsp", fs::copy_options::overwrite_existing);

        Expect(NpdmReader::ReadModuleName(module) == std::string("sysmodule"), "reads name from real exefs.nsp");

        std::optional<std::vector<ModuleInfo>> modules = ModuleScanner::Scan(contents);
        const ModuleInfo* sample = modules ? FindModule(*modules, 0x0100000000000123ull) : nullptr;
        Expect(sample && sample->Name() == "sysmodule", "scanner uses npdm name for unknown module");

        Touch(contents / "0100000000000124" / "exefs.nsp", "PFS0 but truncated");
        Expect(!NpdmReader::ReadModuleName(contents / "0100000000000124").has_value(), "truncated nsp rejected");
    }

    void TestProfiles(const fs::path& root, const fs::path& contents)
    {
        std::optional<std::vector<ModuleInfo>> scanned = ModuleScanner::Scan(contents);
        Expect(scanned.has_value(), "scan for profiles");
        if (!scanned)
            return;
        const std::vector<ModuleInfo>& modules = *scanned;
        fs::path sysClk = contents / "00FF0000636C6BFF";
        fs::path loader = contents / "420000000007E51A";
        BootFlag::Enable(sysClk);
        BootFlag::Enable(loader);

        Profile normal = Profile::Capture("Normal", modules);
        Expect(normal.BootStates().at(0x00FF0000636C6BFFull), "capture records enabled sys-clk");
        Expect(normal.BootStates().at(0x420000000007E51Aull), "capture records enabled loader");

        std::map<std::uint64_t, bool> totkStates{
            { 0x00FF0000636C6BFFull, true },
            { 0x420000000007E51Aull, false },
            { 0x0000000000534C56ull, false },
        };
        Profile totk("TotK", totkStates);
        ApplyResult applied = ProfileApplier::Apply(totk, modules);
        Expect(applied.Changed == 1 && applied.Unchanged == 1 && applied.Missing == 1 && applied.Failed == 0,
            "apply counts changed/unchanged/missing");
        Expect(BootFlag::IsEnabled(sysClk) && !BootFlag::IsEnabled(loader), "apply sets flags on disk");

        ApplyResult reapplied = ProfileApplier::Apply(normal, modules);
        Expect(reapplied.Succeeded() && BootFlag::IsEnabled(loader), "applying normal restores loader");

        fs::path file = root / "config" / "SysToggle" / "profiles.json";
        ProfileStore store(file);
        Expect(!store.Load() && store.Profiles().empty(), "missing profiles file loads empty");
        store.Upsert(normal);
        store.Upsert(totk);
        store.Upsert(Profile("TotK", { { 0x00FF0000636C6BFFull, false } }));
        Expect(store.Profiles().size() == 2, "upsert replaces same name");
        Expect(store.Save(), "profiles saved");

        ProfileStore loaded(file);
        Expect(loaded.Load() && loaded.Profiles().size() == 2, "profiles round trip");
        std::optional<std::size_t> totkIndex = loaded.IndexOf("TotK");
        Expect(totkIndex && loaded.Profiles()[*totkIndex].BootStates().size() == 1, "replaced profile persisted");
        Expect(loaded.Remove("Normal") && !loaded.Remove("Normal"), "remove once");

        Touch(file, R"({"profiles":[{"name":"Ok","modules":{"00FF0000636C6BFF":true,"bad":true,"420000000007E51A":"x"}},{"nope":1}]})");
        ProfileStore messy(file);
        Expect(messy.Load() && messy.Profiles().size() == 1, "invalid profile entries skipped");
        Expect(messy.Profiles().front().BootStates().size() == 1, "invalid module entries skipped");
    }

    void Run(const char* name, const std::function<void()>& test)
    {
        int before = s_Failures;
        test();
        std::printf("%s %s\n", s_Failures == before ? "[ OK ]" : "[FAIL]", name);
    }
}

int main(int argc, char* argv[])
{
    fs::path root = argc > 1 ? fs::path(argv[1]) : fs::temp_directory_path() / "systoggle-tests";
    fs::path sampleNsp = argc > 2 ? fs::path(argv[2]) : fs::path();
    fs::path contents = MakeFixture(root);

    Run("ParseProgramId", TestParseProgramId);
    Run("Scan", [&] { TestScan(contents); });
    Run("ScanMissingDirectory", [&] { TestScanMissingDirectory(root); });
    Run("BootFlag", [&] { TestBootFlag(contents); });
    Run("Settings", [&] { TestSettings(root); });
    Run("Npdm", TestNpdm);
    Run("Profiles", [&] { TestProfiles(root, contents); });
    Run("RealNsp", [&] { TestRealNsp(contents, sampleNsp); });

    fs::remove_all(root);
    std::printf("\n%d failure(s)\n", s_Failures);
    return s_Failures == 0 ? 0 : 1;
}
