module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdtime.time32;

export import pbsd.core;

/// time32 from hbsd/src/lib/libc/stdtime/time32.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status time32_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
