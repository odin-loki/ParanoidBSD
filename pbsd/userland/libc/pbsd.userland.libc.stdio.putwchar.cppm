module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.stdio.putwchar;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/putwchar.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wint_t putwchar_write(wchar_t wc) noexcept { (void)wc; return WEOF; }

} // namespace pbsd::userland::libc
