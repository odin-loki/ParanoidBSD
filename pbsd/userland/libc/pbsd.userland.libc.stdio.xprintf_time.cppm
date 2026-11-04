module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_time;

export import pbsd.core;

/// xprintf_time from hbsd/src/lib/libc/stdio/xprintf_time.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_time_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
