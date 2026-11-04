module;

export module pbsd.kernel.kern_time;

export import pbsd.core;

/// Freestanding port of `kern/kern_time.c` — time helpers.
export namespace pbsd::kernel::kern_time {

inline constexpr int kClockRealtime = 0;
inline constexpr int kClockMonotonic = 3;
inline constexpr int kTimerAbstime = 0x0001;

[[nodiscard]] inline Status validate_clockid(int id) noexcept {
    if (id != kClockRealtime && id != kClockMonotonic) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_absolute(int flags) noexcept {
    return (flags & kTimerAbstime) != 0;
}

} // namespace pbsd::kernel::kern_time
