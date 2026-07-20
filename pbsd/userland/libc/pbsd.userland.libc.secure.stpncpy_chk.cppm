module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.stpncpy_chk;

export import pbsd.core;

/// stpncpy_chk from hbsd/src/lib/libc/secure/stpncpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stpncpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
