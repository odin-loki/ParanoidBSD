module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strncat_chk;

export import pbsd.core;

/// strncat_chk from hbsd/src/lib/libc/secure/strncat_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strncat_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
