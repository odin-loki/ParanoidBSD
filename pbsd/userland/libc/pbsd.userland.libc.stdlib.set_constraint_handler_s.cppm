module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.set_constraint_handler_s;

export import pbsd.core;

/// set_constraint_handler_s from hbsd/src/lib/libc/stdlib/set_constraint_handler_s.c
export namespace pbsd::userland::libc {

inline void set_constraint_handler_s_fn(void* fn) noexcept { (void)fn; }

} // namespace pbsd::userland::libc
