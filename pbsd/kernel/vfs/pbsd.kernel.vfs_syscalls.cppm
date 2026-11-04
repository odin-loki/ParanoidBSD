module;

export module pbsd.kernel.vfs_syscalls;

export import pbsd.core;

/// Freestanding port of `kern/vfs_syscalls.c` — mount/open flags.
export namespace pbsd::kernel::vfs_syscalls {

inline constexpr int kO_RDONLY = 0;
inline constexpr int kO_WRONLY = 1;
inline constexpr int kO_RDWR = 2;
inline constexpr int kO_CREAT = 0x0200;
inline constexpr int kO_EXCL = 0x0800;
inline constexpr int kO_TRUNC = 0x0400;

[[nodiscard]] inline bool wants_create(int flags) noexcept {
    return (flags & kO_CREAT) != 0;
}

[[nodiscard]] inline Status validate_open_flags(int flags) noexcept {
    const int acc = flags & kO_RDWR;
    if (acc != kO_RDONLY && acc != kO_WRONLY && acc != kO_RDWR) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_syscalls
