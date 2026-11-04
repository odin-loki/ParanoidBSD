module;
#include <cstdint>

export module pbsd.arch.arm64.smp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/mp_machdep.c
export namespace pbsd::arch::arm64::smp {

inline constexpr unsigned kMaxCpus = 256;

[[nodiscard]] inline Status validate_mpidr(unsigned mpidr) noexcept {
    return mpidr != 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::smp
