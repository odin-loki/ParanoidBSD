module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcswidth;

export import pbsd.core;

/// wcswidth from hbsd/src/lib/libc/string/wcswidth.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcswidth_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
