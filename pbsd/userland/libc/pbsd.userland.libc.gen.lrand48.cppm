module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.lrand48;

export import pbsd.core;

/// lrand48 from hbsd/src/lib/libc/gen/lrand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long lrand48_val() noexcept { return 0L; }

} // namespace pbsd::userland::libc
