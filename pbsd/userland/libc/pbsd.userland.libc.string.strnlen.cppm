module;
#include <cstddef>

export module pbsd.userland.libc.string.strnlen;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/strnlen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strnlen_count(const char* s, std::size_t maxlen) noexcept { if (s == nullptr) return 0; std::size_t n = 0; while (n < maxlen && s[n] != '\0') ++n; return n; }

} // namespace pbsd::userland::libc
