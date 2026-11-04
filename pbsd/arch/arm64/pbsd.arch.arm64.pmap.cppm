module;
#include <cstdint>

export module pbsd.arch.arm64.pmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/pmap.h — arm64 PTE attributes.
export namespace pbsd::arch::arm64::pmap {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned long long kPageSize = 1ull << kPageShift;

enum class Attr : unsigned long long {
    Valid   = 1ull << 0,
    Table   = 1ull << 1,
    Af      = 1ull << 10,
    Ng      = 1ull << 11,
    Uxn     = 1ull << 54,
    Pxn     = 1ull << 53,
    Cont    = 1ull << 52,
};

enum class Shareability : unsigned char {
    Non     = 0,
    Outer   = 2,
    Inner   = 3,
};

[[nodiscard]] inline Status validate_pte(unsigned long long pte) noexcept {
    if ((pte & static_cast<unsigned long long>(Attr::Valid)) == 0) {
        return Status::Ok;
    }
    if ((pte & static_cast<unsigned long long>(Attr::Uxn))
        && (pte & static_cast<unsigned long long>(Attr::Pxn))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::pmap
