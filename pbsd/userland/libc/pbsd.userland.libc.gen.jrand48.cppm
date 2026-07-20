module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.jrand48;

export import pbsd.core;

/// jrand48 from hbsd/src/lib/libc/gen/jrand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long jrand48_step(unsigned short x[3]) noexcept { return x ? static_cast<long>(x[2]) : 0L; }

} // namespace pbsd::userland::libc
