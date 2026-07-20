module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf;

export import pbsd.core;

/// xprintf from hbsd/src/lib/libc/stdio/xprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
