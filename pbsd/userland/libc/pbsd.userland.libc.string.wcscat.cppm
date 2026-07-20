module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcscat;

export import pbsd.core;

/// wcscat from hbsd/src/lib/libc/string/wcscat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcscat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
