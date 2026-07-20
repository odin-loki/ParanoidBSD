module;
#include <cstdint>

export module pbsd.kernel.subr_prng;

import pbsd.core;

/// Freestanding port of `kern/subr_prng.c` — per-CPU PRNG helpers.
export namespace pbsd::kernel::subr_prng {

[[nodiscard]] inline std::uint32_t mix32(std::uint32_t x) noexcept {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

[[nodiscard]] inline Status validate_bound(std::uint64_t bound) noexcept {
    if (bound == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_prng
