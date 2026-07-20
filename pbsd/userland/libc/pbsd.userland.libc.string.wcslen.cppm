module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wcslen;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/wcslen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t wcslen_count(const wchar_t* s) noexcept { if (s == nullptr) return 0; std::size_t n = 0; while (s[n] != L'\0') ++n; return n; }

} // namespace pbsd::userland::libc
