module;
#include <cstdint>

export module pbsd.bifrost.vmcs;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmcs.h — VMCS field encodings.
export namespace pbsd::bifrost::vmcs {

inline constexpr unsigned long long kInitial = 0xffffffffffffffffull;
inline constexpr unsigned kInvalidEncoding = 0xffffffffu;

enum class Field : unsigned int {
    Vpid              = 0x00000000,
    GuestEsSelector   = 0x00000800,
    GuestCsSelector   = 0x00000802,
    GuestRip          = 0x0000681e,
    GuestRsp          = 0x0000681c,
    ExitReason        = 0x00004402,
    ExitQualification = 0x00006400,
    InstructionError  = 0x00004400,
};

[[nodiscard]] inline unsigned ident(Field f) noexcept {
    return static_cast<unsigned>(f) | 0x80000000u;
}

[[nodiscard]] inline hypervisor::Backend backend_for_vmcs() noexcept {
    return hypervisor::Backend::Vmx;
}

[[nodiscard]] inline Status validate_field(Field f) noexcept {
    if (static_cast<unsigned>(f) == kInvalidEncoding) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vmcs
