module;

export module pbsd.kernel.kern_thr;

export import pbsd.core;

/// Freestanding port of `kern/kern_thr.c` — thr helpers.
export namespace pbsd::kernel::kern_thr {

inline constexpr int kThrCreateSuspended = 0x0001;
inline constexpr int kThrCreateDetached = 0x0002;

[[nodiscard]] inline Status validate_create_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool starts_suspended(int flags) noexcept {
    return (flags & kThrCreateSuspended) != 0;
}

} // namespace pbsd::kernel::kern_thr
