module;
#include <cstddef>

export module pbsd.userland.libc.string.strstr;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/strstr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* strstr_find(const char* haystack, const char* needle) noexcept { (void)haystack; (void)needle; return nullptr; }

} // namespace pbsd::userland::libc
