module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vswprintf;

export import pbsd.core;

/// vswprintf from hbsd/src/lib/libc/stdio/vswprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vswprintf_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
