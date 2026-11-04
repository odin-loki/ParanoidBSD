module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.getopt_long;

export import pbsd.core;

/// getopt_long from hbsd/src/lib/libc/stdlib/getopt_long.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int getopt_long_opt(int argc) noexcept { return argc > 0 ? 0 : -1; }

} // namespace pbsd::userland::libc
