module;
#include <cstdint>

export module pbsd.arch.amd64.smp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/mp_machdep.c
export namespace pbsd::arch::amd64::smp {

inline constexpr unsigned kMaxCpus = 256;

[[nodiscard]] inline Status validate_apic_id(unsigned id) noexcept {
    return id < kMaxCpus ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::smp
