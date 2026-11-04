module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.sprintf_chk;

export import pbsd.core;

/// sprintf_chk from hbsd/src/lib/libc/secure/sprintf_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sprintf_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
