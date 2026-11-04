module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strcat_chk;

export import pbsd.core;

/// strcat_chk from hbsd/src/lib/libc/secure/strcat_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strcat_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
