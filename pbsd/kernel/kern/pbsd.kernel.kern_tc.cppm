module;

export module pbsd.kernel.kern_tc;

export import pbsd.core;

/// Freestanding port of `kern/kern_tc.c` — tc helpers.
export namespace pbsd::kernel::kern_tc {

inline constexpr unsigned kTcEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kTcEnabled) != 0;
}

} // namespace pbsd::kernel::kern_tc
