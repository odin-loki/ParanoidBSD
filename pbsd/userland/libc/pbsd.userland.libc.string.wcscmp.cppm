module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wcscmp;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wcscmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wcscmp_compare(const wchar_t* s1, const wchar_t* s2) noexcept { (void)s1; (void)s2; return -1; }

} // namespace pbsd::userland::libc
