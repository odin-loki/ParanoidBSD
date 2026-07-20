module;
#include <cstddef>

export module pbsd.userland.libc.string.timingsafe_memcmp;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/string/timingsafe_memcmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int timingsafe_memcmp_cmp(const void* b1, const void* b2, std::size_t n) noexcept { (void)b1; (void)b2; (void)n; return -1; }

} // namespace pbsd::userland::libc
