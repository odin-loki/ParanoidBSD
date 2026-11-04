module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.l64a;

export import pbsd.core;

/// l64a from hbsd/src/lib/libc/stdlib/l64a.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* l64a_value(long value) noexcept { (void)value; return ""; }

} // namespace pbsd::userland::libc
