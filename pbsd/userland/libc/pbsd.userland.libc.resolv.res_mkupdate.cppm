module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_mkupdate;

export import pbsd.core;

/// res_mkupdate from hbsd/src/lib/libc/resolv/res_mkupdate.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_mkupdate_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
