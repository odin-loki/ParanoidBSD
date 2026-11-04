module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wmempcpy;

export import pbsd.core;

/// wmempcpy from hbsd/src/lib/libc/string/wmempcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wmempcpy_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
