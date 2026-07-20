module;
#include <cstddef>

export module pbsd.userland.libc.string.strnstr;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/strnstr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* strnstr_find(const char* haystack, const char* needle, std::size_t len) noexcept { (void)haystack; (void)needle; (void)len; return nullptr; }

} // namespace pbsd::userland::libc
