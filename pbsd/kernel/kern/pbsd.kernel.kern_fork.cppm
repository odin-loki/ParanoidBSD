module;

export module pbsd.kernel.kern_fork;

export import pbsd.core;

/// Freestanding port of `kern/kern_fork.c` — fork flags.
export namespace pbsd::kernel::kern_fork {

inline constexpr int kRfproc = 0x0001;
inline constexpr int kRfnowait = 0x0002;
inline constexpr int kRffdg = 0x0004;
inline constexpr int kRfmem = 0x0008;
inline constexpr int kRfposix = 0x0010;

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool returns_child_pid(int flags) noexcept {
    return (flags & kRfproc) != 0;
}

} // namespace pbsd::kernel::kern_fork
