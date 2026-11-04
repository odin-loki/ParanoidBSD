module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.bsearch_b;

export import pbsd.core;

/// bsearch_b from hbsd/src/lib/libc/stdlib/bsearch_b.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const void* bsearch_b_key(const void* key) noexcept { return key; }

} // namespace pbsd::userland::libc
