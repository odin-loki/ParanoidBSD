module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wmemcpy;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wmemcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemcpy_copy(wchar_t* dest, const wchar_t* src, std::size_t n) noexcept { (void)dest; (void)src; (void)n; return dest; }

} // namespace pbsd::userland::libc
