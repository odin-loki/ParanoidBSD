module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.vsnprintf_chk;

export import pbsd.core;

/// vsnprintf_chk from hbsd/src/lib/libc/secure/vsnprintf_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vsnprintf_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
