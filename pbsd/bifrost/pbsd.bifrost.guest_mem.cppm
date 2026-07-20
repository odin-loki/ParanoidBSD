module;
#include <cstdint>

export module pbsd.bifrost.guest_mem;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_mem.c
export namespace pbsd::bifrost::guest_mem {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned kPageSize = 1u << kPageShift;

[[nodiscard]] inline Status validate_gpa(unsigned long long gpa) noexcept {
    return (gpa & (kPageSize - 1)) == 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::guest_mem
