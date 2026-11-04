module;

export module pbsd.kernel.kern_tslog;

export import pbsd.core;

/// Freestanding port of `kern/kern_tslog.c` — tslog helpers.
export namespace pbsd::kernel::kern_tslog {

inline constexpr unsigned kTsMaxRecord = 4096;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kTsMaxRecord ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::kernel::kern_tslog
