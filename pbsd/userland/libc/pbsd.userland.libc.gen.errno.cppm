module;

export module pbsd.userland.libc.gen.errno;

/// errno accessors from hbsd/src/lib/libc/gen/errno.c
export namespace pbsd::userland::libc {

inline thread_local int g_errno{0};

[[nodiscard]] inline int* errno_ptr() noexcept { return &g_errno; }

[[nodiscard]] inline int get_errno() noexcept { return g_errno; }

inline void set_errno(int e) noexcept { g_errno = e; }

} // namespace pbsd::userland::libc
