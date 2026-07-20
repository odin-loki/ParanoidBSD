module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcsncpy;

export import pbsd.core;

/// wcsncpy from hbsd/src/lib/libc/string/wcsncpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcsncpy_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
