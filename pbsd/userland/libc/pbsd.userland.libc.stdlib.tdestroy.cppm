module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.tdestroy;

export import pbsd.core;

/// tdestroy from hbsd/src/lib/libc/stdlib/tdestroy.c
export namespace pbsd::userland::libc {

inline void tdestroy_tree(void* root) noexcept { (void)root; }

} // namespace pbsd::userland::libc
