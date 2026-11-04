module;
#include <cstddef>

export module pbsd.userland.libc.stdlib._Exit;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdlib/_Exit.c
export namespace pbsd::userland::libc {

inline void exit_immediate(int status) noexcept { (void)status; }

} // namespace pbsd::userland::libc
