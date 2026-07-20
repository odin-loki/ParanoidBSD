module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wcscpy;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wcscpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wcscpy_copy(wchar_t* dest, const wchar_t* src) noexcept { if (dest == nullptr || src == nullptr) return dest; wchar_t* d = dest; while ((*d++ = *src++) != L'\0'); return dest; }

} // namespace pbsd::userland::libc
