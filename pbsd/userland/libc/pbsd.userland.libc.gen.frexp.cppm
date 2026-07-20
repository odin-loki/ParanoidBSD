module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.frexp;

export import pbsd.core;

/// frexp from hbsd/src/lib/libc/gen/frexp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double frexp_mantissa(double x, int* exp) noexcept { (void)exp; return x; }

} // namespace pbsd::userland::libc
