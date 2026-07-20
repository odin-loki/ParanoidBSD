module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strlcpy_chk;

export import pbsd.core;

/// strlcpy_chk from hbsd/src/lib/libc/secure/strlcpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strlcpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
