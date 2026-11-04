module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.vsprintf_chk;

export import pbsd.core;

/// vsprintf_chk from hbsd/src/lib/libc/secure/vsprintf_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vsprintf_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
