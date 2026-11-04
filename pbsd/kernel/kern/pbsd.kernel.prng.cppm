module;
#include <cstdint>

export module pbsd.kernel.prng;

export import pbsd.core;

/// Freestanding port of `sys/prng.h` / `kern/subr_prng.c`.
export namespace pbsd::kernel::prng {

[[nodiscard]] inline std::uint32_t bounded(std::uint32_t value, std::uint32_t bound) noexcept {
    if (bound == 0) {
        return 0;
    }
    return value % bound;
}

[[nodiscard]] inline Status validate_bound(std::uint64_t bound) noexcept {
    if (bound == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::prng
