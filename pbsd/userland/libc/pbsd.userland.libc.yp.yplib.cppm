module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.yp.yplib;

export import pbsd.core;

/// yplib from hbsd/src/lib/libc/yp/yplib.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status yplib_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
