module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.hdestroy_r;

export import pbsd.core;

/// hdestroy_r from hbsd/src/lib/libc/stdlib/hdestroy_r.c
export namespace pbsd::userland::libc {

inline void hdestroy_r_table(void* table) noexcept { (void)table; }

} // namespace pbsd::userland::libc
