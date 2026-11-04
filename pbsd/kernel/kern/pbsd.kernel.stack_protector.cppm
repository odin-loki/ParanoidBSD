module;
#include <cstdint>

export module pbsd.kernel.stack_protector;

import pbsd.core;

/// Freestanding port of `kern/stack_protector.c` — stack canary helpers.
export namespace pbsd::kernel::stack_protector {

inline constexpr std::uint64_t kCanaryInit = 0x00000aff0a0d00ffull;

[[nodiscard]] inline std::uint64_t mix_canary(std::uint64_t seed) noexcept {
    seed ^= seed >> 33;
    seed *= 0xff51afd7ed558ccdull;
    seed ^= seed >> 33;
    return seed;
}

[[nodiscard]] inline Status verify_canary(std::uint64_t expected, std::uint64_t actual) noexcept {
    return expected == actual ? Status::Ok : Status::Denied;
}

} // namespace pbsd::kernel::stack_protector
