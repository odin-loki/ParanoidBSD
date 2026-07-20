module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.twalk;

export import pbsd.core;

/// twalk from hbsd/src/lib/libc/stdlib/twalk.c
export namespace pbsd::userland::libc {

inline void twalk_tree(const void* root) noexcept { (void)root; }

} // namespace pbsd::userland::libc
