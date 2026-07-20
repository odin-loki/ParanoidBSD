module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.clock;

export import pbsd.core;

/// clock from hbsd/src/lib/libc/gen/clock.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned long clock_ticks(unsigned long ticks) noexcept { return ticks; }

} // namespace pbsd::userland::libc
