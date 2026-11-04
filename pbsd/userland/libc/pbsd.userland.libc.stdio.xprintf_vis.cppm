module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_vis;

export import pbsd.core;

/// xprintf_vis from hbsd/src/lib/libc/stdio/xprintf_vis.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_vis_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
