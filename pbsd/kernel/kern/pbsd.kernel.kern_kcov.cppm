module;

export module pbsd.kernel.kern_kcov;

export import pbsd.core;

/// Freestanding port of `kern/kern_kcov.c` — kcov helpers.
export namespace pbsd::kernel::kern_kcov {

inline constexpr unsigned kKcovModeTracePc = 0x0001;

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    if (mode & ~kKcovModeTracePc) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_kcov
