module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.strlcat_chk;

export import pbsd.core;

/// strlcat_chk from hbsd/src/lib/libc/secure/strlcat_chk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strlcat_chk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
