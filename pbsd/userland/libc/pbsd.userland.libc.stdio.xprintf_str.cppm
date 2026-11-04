module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_str;

export import pbsd.core;

/// xprintf_str from hbsd/src/lib/libc/stdio/xprintf_str.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_str_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
