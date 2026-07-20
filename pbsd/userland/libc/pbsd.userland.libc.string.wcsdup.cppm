module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wcsdup;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wcsdup.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wcsdup_copy(const wchar_t* s) noexcept { (void)s; return nullptr; }

} // namespace pbsd::userland::libc
