module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.mempcpy_chk;

export import pbsd.core;

/// mempcpy_chk from hbsd/src/lib/libc/secure/mempcpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mempcpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
