module;

export module pbsd.kernel.kern_clocksource;

export import pbsd.core;

/// Freestanding port of `kern/kern_clocksource.c` — clocksource helpers.
export namespace pbsd::kernel::kern_clocksource {

inline constexpr unsigned kCsMonotonic = 0;
inline constexpr unsigned kCsRealtime = 1;

[[nodiscard]] inline Status validate_id(unsigned id) noexcept {
    return id <= kCsRealtime ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_clocksource
