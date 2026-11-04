module;
#include <cstdint>

export module pbsd.arch.amd64.rdrand;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/x86/rdrand.c
export namespace pbsd::arch::amd64::rdrand {

inline constexpr unsigned kEntropyBits = 128;

[[nodiscard]] inline Status validate_entropy_bits(unsigned bits) noexcept {
    return bits == 64 || bits == 128 || bits == 256 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::rdrand
