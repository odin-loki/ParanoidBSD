module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.printf_pos;

export import pbsd.core;

/// printf_pos from hbsd/src/lib/libc/stdio/printf-pos.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status printf_pos_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
