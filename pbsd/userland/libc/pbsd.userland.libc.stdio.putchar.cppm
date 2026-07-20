module;

export module pbsd.userland.libc.stdio.putchar;

/// putchar from hbsd/src/lib/libc/stdio/putchar.c (stdout fd=1 concept)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int putchar(int c) noexcept { return c; }

} // namespace pbsd::userland::libc::stdio
