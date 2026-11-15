#pragma once

#include <cstdint>

namespace SysToggle::Core
{
    enum class ProcessState
    {
        Running,
        Stopped,
        Unknown,
    };

    class ProcessControl
    {
    public:
        ProcessControl();
        ~ProcessControl();
        ProcessControl(const ProcessControl&) = delete;
        ProcessControl& operator=(const ProcessControl&) = delete;

        bool IsAvailable() const { return m_IsAvailable; }

        ProcessState QueryState(std::uint64_t programId) const;
        bool Launch(std::uint64_t programId) const;
        bool Terminate(std::uint64_t programId) const;

    private:
        bool m_IsAvailable;
    };
}
