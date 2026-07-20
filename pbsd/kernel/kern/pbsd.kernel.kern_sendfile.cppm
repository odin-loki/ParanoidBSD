module;

export module pbsd.kernel.kern_sendfile;

export import pbsd.core;

/// Freestanding port of `kern/kern_sendfile.c` — sendfile helpers.
export namespace pbsd::kernel::kern_sendfile {

inline constexpr int kSfMntWait = 0x0001;
inline constexpr int kSfSync = 0x0002;
inline constexpr int kSfNodiskio = 0x0004;

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool wants_sync(int flags) noexcept {
    return (flags & kSfSync) != 0;
}

} // namespace pbsd::kernel::kern_sendfile
