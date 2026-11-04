module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.tdelete;

export import pbsd.core;

/// tdelete from hbsd/src/lib/libc/stdlib/tdelete.c
export namespace pbsd::userland::libc {

inline void tdelete_node(void* root) noexcept { (void)root; }

} // namespace pbsd::userland::libc
