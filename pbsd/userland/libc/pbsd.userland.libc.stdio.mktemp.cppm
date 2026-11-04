module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.mktemp;

export import pbsd.core;

/// mktemp from hbsd/src/lib/libc/stdio/mktemp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mktemp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
