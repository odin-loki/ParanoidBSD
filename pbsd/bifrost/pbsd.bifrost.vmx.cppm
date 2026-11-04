module;
#include <cstdint>

export module pbsd.bifrost.vmx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmx_controls.h, vmcs.h
export namespace pbsd::bifrost::vmx {

inline constexpr std::uint32_t kPinExtIntExiting = 1u << 0;
inline constexpr std::uint32_t kPinNmiExiting = 1u << 3;
inline constexpr std::uint32_t kPinPreemptionTimer = 1u << 6;

inline constexpr std::uint32_t kProcHltExiting = 1u << 7;
inline constexpr std::uint32_t kProcInvlpgExiting = 1u << 9;
inline constexpr std::uint32_t kProcIoExiting = 1u << 24;
inline constexpr std::uint32_t kProcMsrBitmaps = 1u << 28;
inline constexpr std::uint32_t kProcSecondaryControls = 1u << 31;

inline constexpr std::uint32_t kProc2EnableEpt = 1u << 1;
inline constexpr std::uint32_t kProc2EnableVpid = 1u << 5;
inline constexpr std::uint32_t kProc2UnrestrictedGuest = 1u << 7;

inline constexpr std::uint32_t kVmExitHostLma = 1u << 9;
inline constexpr std::uint32_t kVmExitAckIntr = 1u << 15;

struct ControlSet {
    std::uint32_t pin{0};
    std::uint32_t proc{0};
    std::uint32_t proc2{0};
    std::uint32_t exit{0};
};

[[nodiscard]] inline Status validate_controls(const ControlSet& c) noexcept {
    if ((c.proc & kProcSecondaryControls) != 0 && c.proc2 == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vmx
