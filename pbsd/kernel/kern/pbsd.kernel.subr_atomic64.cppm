module;
#include <cstdint>

export module pbsd.kernel.subr_atomic64;

export import pbsd.core;

/// Freestanding port of `kern/subr_atomic64.c` — atomic64 helpers.
export namespace pbsd::kernel::subr_atomic64 {

[[nodiscard]] inline std::uint64_t fetch_add(std::uint64_t& v, std::uint64_t delta) noexcept {
    const auto old = v;
    v += delta;
    return old;
}

[[nodiscard]] inline bool compare_exchange(std::uint64_t& v, std::uint64_t expected,
                                           std::uint64_t desired) noexcept {
    if (v != expected) {
        return false;
    }
    v = desired;
    return true;
}

} // namespace pbsd::kernel::subr_atomic64
