module;
#include <cstdint>

export module pbsd.kernel.subr_counter;

import pbsd.core;
import pbsd.kernel.counter;

/// Freestanding port of `kern/subr_counter.c` — counter(9) registration helpers.
export namespace pbsd::kernel::subr_counter {

[[nodiscard]] inline Status register_u64(counter::U64Counter& c) noexcept {
    counter::zero(c);
    return Status::Ok;
}

[[nodiscard]] inline Status rate_limit(counter::RateLimiter& r,
                                       std::int64_t now) noexcept {
    return counter::rate_check(r, now);
}

template<unsigned N>
[[nodiscard]] inline Status snapshot(const counter::CounterArray<N>& src,
                                   counter::CounterArray<N>& dst) noexcept {
    counter::array_copy(src, dst);
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_counter
