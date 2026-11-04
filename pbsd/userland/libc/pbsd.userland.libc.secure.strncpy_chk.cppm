module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strncpy_chk;

export import pbsd.core;

/// strncpy_chk from hbsd/src/lib/libc/secure/strncpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strncpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
