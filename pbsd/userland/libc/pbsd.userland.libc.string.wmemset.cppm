module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wmemset;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wmemset.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemset_fill(wchar_t* s, wchar_t c, std::size_t n) noexcept { (void)s; (void)c; (void)n; return s; }

} // namespace pbsd::userland::libc
