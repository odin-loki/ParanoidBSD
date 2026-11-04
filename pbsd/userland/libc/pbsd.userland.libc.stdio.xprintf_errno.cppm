module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_errno;

export import pbsd.core;

/// xprintf_errno from hbsd/src/lib/libc/stdio/xprintf_errno.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_errno_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
