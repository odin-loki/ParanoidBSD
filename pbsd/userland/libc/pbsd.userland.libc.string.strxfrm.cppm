module;
#include <cstddef>

export module pbsd.userland.libc.string.strxfrm;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/strxfrm.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strxfrm_count(char* dest, const char* src, std::size_t n) noexcept { (void)dest; (void)src; (void)n; return 0; }

} // namespace pbsd::userland::libc
