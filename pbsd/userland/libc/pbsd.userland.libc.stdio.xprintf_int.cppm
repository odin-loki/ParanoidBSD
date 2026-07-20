module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_int;

export import pbsd.core;

/// xprintf_int from hbsd/src/lib/libc/stdio/xprintf_int.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_int_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
