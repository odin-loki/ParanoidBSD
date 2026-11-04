module;

export module pbsd.userland.libc.sys.close;

export import pbsd.core;

/// close from hbsd/src/lib/libc/sys/close.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status close_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
