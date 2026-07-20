module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wmemcmp;

export import pbsd.core;

/// wmemcmp from hbsd/src/lib/libc/string/wmemcmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wmemcmp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
