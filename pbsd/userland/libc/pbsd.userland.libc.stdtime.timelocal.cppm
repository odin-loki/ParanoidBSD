module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdtime.timelocal;

export import pbsd.core;

/// timelocal from hbsd/src/lib/libc/stdtime/timelocal.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status timelocal_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
