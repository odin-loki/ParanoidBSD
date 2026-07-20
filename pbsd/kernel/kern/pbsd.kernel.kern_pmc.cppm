module;

export module pbsd.kernel.kern_pmc;

export import pbsd.core;

/// Freestanding port of `kern/kern_pmc.c` — pmc helpers.
export namespace pbsd::kernel::kern_pmc {

inline constexpr unsigned kPmcEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kPmcEnabled) != 0;
}

} // namespace pbsd::kernel::kern_pmc
