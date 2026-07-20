module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_hexdump;

export import pbsd.core;

/// xprintf_hexdump from hbsd/src/lib/libc/stdio/xprintf_hexdump.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_hexdump_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
