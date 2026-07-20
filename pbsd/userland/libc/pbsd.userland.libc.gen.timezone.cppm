module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.timezone;

export import pbsd.core;

/// timezone from hbsd/src/lib/libc/gen/timezone.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long timezone_offset() noexcept { return 0L; }

} // namespace pbsd::userland::libc
