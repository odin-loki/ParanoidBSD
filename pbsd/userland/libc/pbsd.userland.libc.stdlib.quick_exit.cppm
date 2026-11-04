module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.quick_exit;

export import pbsd.core;

/// quick_exit from hbsd/src/lib/libc/stdlib/quick_exit.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int quick_exit_code(int status) noexcept { return status; }

} // namespace pbsd::userland::libc
