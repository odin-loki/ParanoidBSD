module;

export module pbsd.kernel.kern_priv;

export import pbsd.core;

/// Freestanding port of `kern/kern_priv.c` — priv helpers.
export namespace pbsd::kernel::kern_priv {

inline constexpr unsigned kPrivAll = 0xffffffffu;

[[nodiscard]] inline Status validate_set(unsigned priv) noexcept {
    return priv <= kPrivAll ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_priv
