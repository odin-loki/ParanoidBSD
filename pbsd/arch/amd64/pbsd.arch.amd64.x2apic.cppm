module;
#include <cstdint>

export module pbsd.arch.amd64.x2apic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/x2apic.c
export namespace pbsd::arch::amd64::x2apic {

inline constexpr unsigned kBaseMsr = 0x800;
inline constexpr unsigned kMaxId = 255;

[[nodiscard]] inline Status validate_apic_id(unsigned id) noexcept {
    return id <= kMaxId ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_msr(unsigned msr) noexcept {
    if (msr < kBaseMsr || msr > kBaseMsr + kMaxId) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::x2apic
