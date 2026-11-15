#include "core/ProcessControl.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace SysToggle::Core
{
#ifdef __SWITCH__

    ProcessControl::ProcessControl()
        : m_IsAvailable(false)
    {
        bool dmntReady = R_SUCCEEDED(pmdmntInitialize());
        bool shellReady = R_SUCCEEDED(pmshellInitialize());
        m_IsAvailable = dmntReady && shellReady;
    }

    ProcessControl::~ProcessControl()
    {
        pmshellExit();
        pmdmntExit();
    }

    ProcessState ProcessControl::QueryState(std::uint64_t programId) const
    {
        if (!m_IsAvailable)
            return ProcessState::Unknown;
        u64 processId = 0;
        if (R_SUCCEEDED(pmdmntGetProcessId(&processId, programId)) && processId != 0)
            return ProcessState::Running;
        return ProcessState::Stopped;
    }

    bool ProcessControl::Launch(std::uint64_t programId) const
    {
        if (!m_IsAvailable)
            return false;
        NcmProgramLocation location{};
        location.program_id = programId;
        location.storageID = NcmStorageId_None;
        u64 processId = 0;
        return R_SUCCEEDED(pmshellLaunchProgram(0, &location, &processId));
    }

    bool ProcessControl::Terminate(std::uint64_t programId) const
    {
        if (!m_IsAvailable)
            return false;
        return R_SUCCEEDED(pmshellTerminateProgram(programId));
    }

#else

    ProcessControl::ProcessControl()
        : m_IsAvailable(false)
    {
    }

    ProcessControl::~ProcessControl() = default;

    ProcessState ProcessControl::QueryState(std::uint64_t) const
    {
        return ProcessState::Unknown;
    }

    bool ProcessControl::Launch(std::uint64_t) const
    {
        return false;
    }

    bool ProcessControl::Terminate(std::uint64_t) const
    {
        return false;
    }

#endif
}
