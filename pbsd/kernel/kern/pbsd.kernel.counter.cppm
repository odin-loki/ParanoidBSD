module;
#include <cstdint>

export module pbsd.kernel.counter;

import pbsd.core;

/// Freestanding port of `sys/counter.h` rate-limit and u64 counter helpers.
export namespace pbsd::kernel::counter {

inline constexpr int kWaitOk   = 0;
inline constexpr int kWaitFail = 1;

struct RateLimiter {
    int     period_sec{};
    std::int64_t last_check{};
    std::uint64_t count{};
    std::uint64_t limit{};
};

[[nodiscard]] inline Status validate_period(int period_sec) noexcept {
    if (period_sec <= 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init_rate(RateLimiter& r, int period_sec,
                                    std::uint64_t limit) noexcept {
    if (validate_period(period_sec) != Status::Ok || limit == 0) {
        return Status::Invalid;
    }
    r.period_sec = period_sec;
    r.limit = limit;
    r.count = 0;
    r.last_check = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status rate_check(RateLimiter& r, std::int64_t now) noexcept {
    if (r.period_sec <= 0) {
        return Status::Invalid;
    }
    if (now - r.last_check >= r.period_sec) {
        r.count = 0;
        r.last_check = now;
    }
    if (r.count >= r.limit) {
        return Status::Denied;
    }
    ++r.count;
    return Status::Ok;
}

struct U64Counter {
    std::uint64_t value{};
};

inline void increment(U64Counter& c) noexcept {
    ++c.value;
}

inline void add(U64Counter& c, std::uint64_t delta) noexcept {
    c.value += delta;
}

[[nodiscard]] inline std::uint64_t read(const U64Counter& c) noexcept {
    return c.value;
}

inline void zero(U64Counter& c) noexcept {
    c.value = 0;
}

template<unsigned N>
struct CounterArray {
    U64Counter cells[N]{};
};

template<unsigned N>
inline void array_zero(CounterArray<N>& arr) noexcept {
    for (unsigned i = 0; i < N; ++i) {
        zero(arr.cells[i]);
    }
}

template<unsigned N>
inline void array_copy(const CounterArray<N>& src,
                       CounterArray<N>& dst) noexcept {
    for (unsigned i = 0; i < N; ++i) {
        dst.cells[i].value = src.cells[i].value;
    }
}

} // namespace pbsd::kernel::counter
