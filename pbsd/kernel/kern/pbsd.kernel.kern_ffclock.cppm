module;

export module pbsd.kernel.kern_ffclock;

export import pbsd.core;

/// Freestanding port of `kern/kern_ffclock.c` — ffclock helpers.
export namespace pbsd::kernel::kern_ffclock {

inline constexpr unsigned kFfActive = 0x0001;

[[nodiscard]] inline bool is_active(unsigned state) noexcept {
    return (state & kFfActive) != 0;
}

} // namespace pbsd::kernel::kern_ffclock
