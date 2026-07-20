module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.wsetup;

export import pbsd.core;

/// wsetup from hbsd/src/lib/libc/stdio/wsetup.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wsetup_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
