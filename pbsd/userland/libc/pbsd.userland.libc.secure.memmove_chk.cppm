module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.memmove_chk;

export import pbsd.core;

/// memmove_chk from hbsd/src/lib/libc/secure/memmove_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status memmove_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
