module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcsstr;

export import pbsd.core;

/// wcsstr from hbsd/src/lib/libc/string/wcsstr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcsstr_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
