module;

export module pbsd.kernel.kern_boottrace;

export import pbsd.core;

/// Freestanding port of `kern/kern_boottrace.c` — boottrace helpers.
export namespace pbsd::kernel::kern_boottrace {

inline constexpr unsigned kBtEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kBtEnabled) != 0;
}

} // namespace pbsd::kernel::kern_boottrace
