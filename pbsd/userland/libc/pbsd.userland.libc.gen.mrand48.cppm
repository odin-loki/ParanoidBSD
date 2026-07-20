module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.mrand48;

export import pbsd.core;

/// mrand48 from hbsd/src/lib/libc/gen/mrand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long mrand48_val() noexcept { return 0L; }

} // namespace pbsd::userland::libc
