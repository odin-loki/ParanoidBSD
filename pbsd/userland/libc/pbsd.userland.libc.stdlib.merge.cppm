module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.merge;

export import pbsd.core;

/// merge from hbsd/src/lib/libc/stdlib/merge.c
export namespace pbsd::userland::libc {

inline void merge_arrays(void* base1, void* base2, std::size_t nmemb, std::size_t size, int (*compar)(const void*, const void*)) noexcept { (void)base1; (void)base2; (void)nmemb; (void)size; (void)compar; }

} // namespace pbsd::userland::libc
