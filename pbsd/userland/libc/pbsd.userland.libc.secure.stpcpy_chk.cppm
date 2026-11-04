module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.stpcpy_chk;

export import pbsd.core;

/// stpcpy_chk from hbsd/src/lib/libc/secure/stpcpy_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stpcpy_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
