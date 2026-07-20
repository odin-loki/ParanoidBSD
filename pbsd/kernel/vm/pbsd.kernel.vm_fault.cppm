module;
#include <cstdint>

export module pbsd.kernel.vm_fault;

import pbsd.core;
import pbsd.kernel.vm;

/// Freestanding port of `vm_fault.c` fault disposition codes + policy bits.
export namespace pbsd::kernel::vm_fault {

using namespace pbsd::kernel::vm;

inline constexpr int kVmFaultReadDefault = 2; // 1 + VM_FAULT_READ_AHEAD_INIT
inline constexpr int kVmFaultDontneedMin  = 1048576;

enum class FaultResult : int {
    Success             = 10000,
    Failure,
    Continue,
    Restart,
    OutOfBounds,
    Hard,
    Soft,
    ProtectionFailure,
};

enum class FaultNext : int {
    GotObj  = 1,
    NoObj,
    Restart,
};

inline constexpr unsigned kFaultFlagDirty = 0x0001u;
inline constexpr unsigned kFaultFlagWire  = 0x0002u;
inline constexpr unsigned kFaultFlagSlept = 0x0004u;

struct FaultState {
    unsigned char fault_type{kVmProtRead};
    unsigned      fault_flags{};
    unsigned char max_prot{kVmProtAll};
};

[[nodiscard]] constexpr Status to_status(FaultResult r) noexcept {
    switch (r) {
    case FaultResult::Success:
    case FaultResult::Soft:
    case FaultResult::Hard:
        return Status::Ok;
    case FaultResult::ProtectionFailure:
        return Status::Denied;
    case FaultResult::OutOfBounds:
        return Status::NotFound;
    case FaultResult::Failure:
    case FaultResult::Restart:
    case FaultResult::Continue:
        return Status::Protocol;
    }
    return Status::Protocol;
}

/// `vm_fault_wire_check` — wired faults skip dirty promotion unless flagged.
[[nodiscard]] constexpr bool wire_allows_dirty(const FaultState& fs) noexcept {
    if ((fs.fault_flags & kFaultFlagWire) == 0) {
        return true;
    }
    return (fs.fault_flags & kFaultFlagDirty) != 0;
}

/// `vm_fault_protect` — reject W^X faults early.
[[nodiscard]] inline Status validate_fault_prot(unsigned char prot) noexcept {
    return validate_wx_vm(prot);
}

/// Map pager-style result to fault result — vm_fault.c return path.
[[nodiscard]] constexpr FaultResult from_pager_ok(bool ok, bool protection) noexcept {
    if (protection) {
        return FaultResult::ProtectionFailure;
    }
    return ok ? FaultResult::Success : FaultResult::Failure;
}

} // namespace pbsd::kernel::vm_fault
