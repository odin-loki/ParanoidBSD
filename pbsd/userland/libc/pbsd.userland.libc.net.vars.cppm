module;

export module pbsd.userland.libc.net.vars;

/// h_errno / net vars from hbsd/src/lib/libc/net/vars.c
export namespace pbsd::userland::libc {

inline int g_h_errno = 0;

[[nodiscard]] inline int h_errno_value() noexcept { return g_h_errno; }

inline void set_h_errno(int e) noexcept { g_h_errno = e; }

} // namespace pbsd::userland::libc
