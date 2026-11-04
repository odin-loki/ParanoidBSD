module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.tempnam;

export import pbsd.core;

/// tempnam from hbsd/src/lib/libc/stdio/tempnam.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status tempnam_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
