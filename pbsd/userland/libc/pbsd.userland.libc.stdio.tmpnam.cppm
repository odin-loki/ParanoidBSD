module;
#include <cstddef>

export module pbsd.userland.libc.stdio.tmpnam;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/tmpnam.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* tmpnam_path(char* s) noexcept { (void)s; return nullptr; }

} // namespace pbsd::userland::libc
