module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.memcpy_chk;

export import pbsd.core;

/// memcpy_chk from hbsd/src/lib/libc/secure/memcpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status memcpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
