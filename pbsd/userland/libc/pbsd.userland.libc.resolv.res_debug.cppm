module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_debug;

export import pbsd.core;

/// res_debug from hbsd/src/lib/libc/resolv/res_debug.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_debug_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
