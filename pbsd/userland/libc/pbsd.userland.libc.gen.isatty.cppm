module;

export module pbsd.userland.libc.gen.isatty;

/// isatty/ttyslot concepts from hbsd/src/lib/libc/gen/{isatty,ttyslot}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool isatty_fd(int fd) noexcept {
    return fd >= 0 && fd <= 2;
}

[[nodiscard]] inline int ttyslot_from_fd(int fd) noexcept {
    return isatty_fd(fd) ? fd + 1 : 0;
}

} // namespace pbsd::userland::libc
