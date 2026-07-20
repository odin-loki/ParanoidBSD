module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.nrand48;

export import pbsd.core;

/// nrand48 from hbsd/src/lib/libc/gen/nrand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long nrand48_step(unsigned short x[3]) noexcept { return x ? static_cast<long>(x[0]) : 0L; }

} // namespace pbsd::userland::libc
