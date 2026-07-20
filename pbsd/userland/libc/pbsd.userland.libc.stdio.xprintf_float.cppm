module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_float;

export import pbsd.core;

/// xprintf_float from hbsd/src/lib/libc/stdio/xprintf_float.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_float_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
