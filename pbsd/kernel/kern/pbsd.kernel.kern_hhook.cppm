module;

export module pbsd.kernel.kern_hhook;

export import pbsd.core;

/// Freestanding port of `kern/kern_hhook.c` — hhook helpers.
export namespace pbsd::kernel::kern_hhook {

inline constexpr unsigned kHhMaxHooks = 32;

[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    return idx < kHhMaxHooks ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_hhook
