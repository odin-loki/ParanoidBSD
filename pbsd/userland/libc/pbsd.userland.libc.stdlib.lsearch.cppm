module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.lsearch;

export import pbsd.core;

/// lsearch from hbsd/src/lib/libc/stdlib/lsearch.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* lsearch_key(const void* key, void* base, std::size_t* nelp, std::size_t width, int (*compar)(const void*, const void*)) noexcept { (void)key; (void)base; (void)nelp; (void)width; (void)compar; return nullptr; }

} // namespace pbsd::userland::libc
