module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_update;

export import pbsd.core;

/// res_update from hbsd/src/lib/libc/resolv/res_update.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_update_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
