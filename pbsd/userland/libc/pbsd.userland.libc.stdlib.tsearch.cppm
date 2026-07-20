module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.tsearch;

export import pbsd.core;

/// tsearch from hbsd/src/lib/libc/stdlib/tsearch.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void** tsearch_key(const void* key) noexcept { (void)key; return nullptr; }

} // namespace pbsd::userland::libc
