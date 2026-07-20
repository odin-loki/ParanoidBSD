module;

export module pbsd.userland.libc.stdio.getchar;

/// getchar from hbsd/src/lib/libc/stdio/getchar.c (stdin fd=0 concept)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int getchar() noexcept { return -1; }

} // namespace pbsd::userland::libc::stdio
