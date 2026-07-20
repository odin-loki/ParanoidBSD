module;

export module pbsd.userland.libc.gen.closedir;

export import pbsd.core;

/// closedir from hbsd/src/lib/libc/gen/closedir.c
export namespace pbsd::userland::libc {

struct DirHandle { int fd{-1}; };

[[nodiscard]] inline Status closedir_handle(DirHandle& d) noexcept {
    if (d.fd < 0) {
        return Status::Invalid;
    }
    d.fd = -1;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
