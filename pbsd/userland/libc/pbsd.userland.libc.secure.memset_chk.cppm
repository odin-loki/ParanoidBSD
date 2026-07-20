module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.memset_chk;

export import pbsd.core;

/// memset_chk from hbsd/src/lib/libc/secure/memset_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status memset_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
