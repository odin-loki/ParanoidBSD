module;

export module pbsd.userland.libc.gen.dirfd;

/// dirfd from hbsd/src/lib/libc/gen/dirfd.c
export namespace pbsd::userland::libc {

struct Dir {
    int fd{-1};
};

[[nodiscard]] inline int dirfd(const Dir& d) noexcept { return d.fd; }

} // namespace pbsd::userland::libc
