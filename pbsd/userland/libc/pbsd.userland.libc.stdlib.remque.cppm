module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.remque;

export import pbsd.core;

/// remque from hbsd/src/lib/libc/stdlib/remque.c
export namespace pbsd::userland::libc {

inline void remque_node(void* node) noexcept { (void)node; }

} // namespace pbsd::userland::libc
