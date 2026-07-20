module;
#include <cstdint>

export module pbsd.arch.amd64.avx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/avx.c
export namespace pbsd::arch::amd64::avx {

enum class Level : unsigned char {
    None = 0,
    Avx = 1,
    Avx2 = 2,
};

[[nodiscard]] inline Status validate_level(unsigned level) noexcept {
    return level <= static_cast<unsigned>(Level::Avx2) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::avx
