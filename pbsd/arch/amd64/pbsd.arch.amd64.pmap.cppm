module;
#include <cstdint>

export module pbsd.arch.amd64.pmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/pmap.h — page table entry bits.
export namespace pbsd::arch::amd64::pmap {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned long long kPageSize = 1ull << kPageShift;

enum class PteFlag : unsigned long long {
    Present  = 1ull << 0,
    Write    = 1ull << 1,
    User     = 1ull << 2,
    Pwt      = 1ull << 3,
    Pcd      = 1ull << 4,
    Accessed = 1ull << 5,
    Dirty    = 1ull << 6,
    Large    = 1ull << 7,
    Global   = 1ull << 8,
    Nx       = 1ull << 63,
};

[[nodiscard]] inline Status validate_pte(unsigned long long pte) noexcept {
    if ((pte & static_cast<unsigned long long>(PteFlag::Present)) == 0) {
        return Status::Ok;
    }
    if ((pte & static_cast<unsigned long long>(PteFlag::Large))
        && (pte & static_cast<unsigned long long>(PteFlag::Dirty))) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::pmap
