module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.random;

export import pbsd.core;

/// random from hbsd/src/lib/libc/stdlib/random.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long random_val() noexcept { return 0L; }

} // namespace pbsd::userland::libc
