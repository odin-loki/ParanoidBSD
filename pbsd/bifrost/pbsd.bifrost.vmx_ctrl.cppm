module;
#include <cstdint>

export module pbsd.bifrost.vmx_ctrl;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmx_controls.h — VMX execution/exit controls.
export namespace pbsd::bifrost::vmx_ctrl {

enum class PinFlag : unsigned int {
    ExtIntExit = 1u << 0,
    NmiExit = 1u << 3,
    VirtualNmi = 1u << 5,
    PreemptionTimer = 1u << 6,
};

enum class ProcFlag : unsigned int {
    HltExit = 1u << 7,
    RdtscExit = 1u << 12,
    IoExit = 1u << 24,
    MsrBitmap = 1u << 28,
    Secondary = 1u << 31,
};

enum class Proc2Flag : unsigned int {
    EnableEpt = 1u << 1,
    EnableVpid = 1u << 5,
    UnrestrictedGuest = 1u << 7,
    EnableInvpcid = 1u << 12,
};

[[nodiscard]] inline Status validate_pin(unsigned ctrl) noexcept {
    (void)ctrl;
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend() noexcept {
    return hypervisor::Backend::Vmx;
}

} // namespace pbsd::bifrost::vmx_ctrl
