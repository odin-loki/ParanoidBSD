module;
#include <cstdint>

export module pbsd.arch.amd64.pat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/pat.c
export namespace pbsd::arch::amd64::pat {

enum class Entry : unsigned char {
    Uncacheable = 0,
    WriteCombining = 1,
    WriteThrough = 4,
    WriteProtected = 5,
    WriteBack = 6,
};

inline constexpr unsigned kEntryCount = 8;

[[nodiscard]] inline Status validate_entry(unsigned e) noexcept {
    return e < kEntryCount ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::pat
