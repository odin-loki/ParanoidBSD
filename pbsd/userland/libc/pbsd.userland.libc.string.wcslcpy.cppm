module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcslcpy;

export import pbsd.core;

/// wcslcpy from hbsd/src/lib/libc/string/wcslcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcslcpy_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
