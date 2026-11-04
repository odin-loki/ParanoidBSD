module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.seed48;

export import pbsd.core;

/// seed48 from hbsd/src/lib/libc/gen/seed48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned short* seed48_state(unsigned short x[3]) noexcept { return x; }

} // namespace pbsd::userland::libc
