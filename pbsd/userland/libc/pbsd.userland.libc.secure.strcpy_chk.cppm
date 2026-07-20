module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strcpy_chk;

export import pbsd.core;

/// strcpy_chk from hbsd/src/lib/libc/secure/strcpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strcpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
