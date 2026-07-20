module;

#include <cstdint>

export module pbsd.userland.libc.gen.time;

/// time(3) epoch helpers from hbsd/src/lib/libc/gen/time.c (logic-only)
export namespace pbsd::userland::libc {

using TimeT = std::int64_t;

[[nodiscard]] inline TimeT time_now(TimeT* out) noexcept {
    const TimeT t = 0;
    if (out != nullptr) {
        *out = t;
    }
    return t;
}

[[nodiscard]] inline bool is_leap_year(int year) noexcept {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

[[nodiscard]] inline int days_in_month(int year, int month) noexcept {
    static constexpr int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 1 && is_leap_year(year)) {
        return 29;
    }
    return kDays[month];
}

} // namespace pbsd::userland::libc
