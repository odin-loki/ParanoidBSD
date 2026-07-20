module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.clock_nanosleep;

export import pbsd.core;

/// clock_nanosleep from hbsd/src/lib/libc/sys/clock_nanosleep.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status clock_nanosleep_id(int clock_id) noexcept { return clock_id >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
