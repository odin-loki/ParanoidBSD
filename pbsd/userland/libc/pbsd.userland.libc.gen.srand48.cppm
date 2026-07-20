module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.srand48;

export import pbsd.core;

/// srand48 from hbsd/src/lib/libc/gen/srand48.c
export namespace pbsd::userland::libc {

inline void srand48_seed(long seed) noexcept { (void)seed; }

} // namespace pbsd::userland::libc
