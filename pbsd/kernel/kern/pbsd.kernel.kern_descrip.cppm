module;

export module pbsd.kernel.kern_descrip;

export import pbsd.core;

/// Freestanding port of `kern/kern_descrip.c` — descrip helpers.
export namespace pbsd::kernel::kern_descrip {

inline constexpr int kMaxFd = 1 << 20;
inline constexpr int kDupFixed = 0x0001;

[[nodiscard]] inline Status validate_fd(int fd) noexcept {
    if (fd < 0 || fd >= kMaxFd) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_dup2(int oldfd, int newfd) noexcept {
    if (validate_fd(oldfd) != Status::Ok || validate_fd(newfd) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_descrip
