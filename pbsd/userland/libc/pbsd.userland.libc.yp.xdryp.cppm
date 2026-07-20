module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.yp.xdryp;

export import pbsd.core;

/// xdryp from hbsd/src/lib/libc/yp/xdryp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xdryp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
