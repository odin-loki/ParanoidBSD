module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.fgets_chk;

export import pbsd.core;

/// fgets_chk from hbsd/src/lib/libc/secure/fgets_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgets_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
