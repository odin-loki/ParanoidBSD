module;
#include <cstdint>

export module pbsd.arch.amd64.mtrr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/mtrr.c
export namespace pbsd::arch::amd64::mtrr {

enum class Type : unsigned char {
    Uncacheable = 0,
    WriteCombining = 1,
    WriteThrough = 4,
    WriteProtected = 5,
    WriteBack = 6,
};

inline constexpr unsigned kMaxRegions = 8;

[[nodiscard]] inline Status validate_type(unsigned t) noexcept {
    return t <= static_cast<unsigned>(Type::WriteBack) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::mtrr
