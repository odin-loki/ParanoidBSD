module;
#include <cstddef>

export module pbsd.userland.libc.stdio.makebuf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/makebuf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status makebuf_init(int fd) noexcept { if (fd < 0) return Status::Invalid; return Status::NotImplemented; }

} // namespace pbsd::userland::libc
