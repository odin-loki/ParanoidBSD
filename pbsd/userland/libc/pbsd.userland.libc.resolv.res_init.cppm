module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_init;

export import pbsd.core;

/// res_init from hbsd/src/lib/libc/resolv/res_init.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_init_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
