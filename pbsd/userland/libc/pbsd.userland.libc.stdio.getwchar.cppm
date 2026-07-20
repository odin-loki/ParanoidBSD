module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.stdio.getwchar;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/getwchar.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wint_t getwchar_read() noexcept { return WEOF; }

} // namespace pbsd::userland::libc
