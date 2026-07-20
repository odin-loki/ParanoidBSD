module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.stdio.swprintf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/swprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int swprintf_fmt(wchar_t* ws, std::size_t n, const wchar_t* fmt) noexcept { (void)ws; (void)n; (void)fmt; return -1; }

} // namespace pbsd::userland::libc
