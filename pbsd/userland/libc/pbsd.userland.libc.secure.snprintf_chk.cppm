module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.snprintf_chk;

export import pbsd.core;

/// snprintf_chk from hbsd/src/lib/libc/secure/snprintf_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status snprintf_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
