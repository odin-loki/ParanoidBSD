module;
#include <cstdint>

export module pbsd.kernel.fattime;

export import pbsd.core;

/// Freestanding port of `kern/subr_fattime.c` — FAT timestamp packing.
export namespace pbsd::kernel::fattime {

inline constexpr unsigned kSecondsPerMinute = 60;
inline constexpr unsigned kMinutesPerHour = 60;
inline constexpr unsigned kHoursPerDay = 24;
inline constexpr unsigned kDaysPerMonth = 31;
inline constexpr unsigned kMonthsPerYear = 12;
inline constexpr unsigned kYearBase = 1980;

[[nodiscard]] inline Status validate_fields(unsigned sec, unsigned min, unsigned hour,
                                            unsigned day, unsigned mon, unsigned year) noexcept {
    if (sec >= kSecondsPerMinute || min >= kMinutesPerHour || hour >= kHoursPerDay) {
        return Status::Invalid;
    }
    if (day == 0 || day > kDaysPerMonth || mon == 0 || mon > kMonthsPerYear) {
        return Status::Invalid;
    }
    if (year < kYearBase) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint32_t pack_time(unsigned sec, unsigned min, unsigned hour) noexcept {
    return (hour << 11) | (min << 5) | (sec / 2);
}

[[nodiscard]] inline std::uint16_t pack_date(unsigned day, unsigned mon, unsigned year) noexcept {
    return static_cast<std::uint16_t>(((year - kYearBase) << 9) | (mon << 5) | day);
}

} // namespace pbsd::kernel::fattime
