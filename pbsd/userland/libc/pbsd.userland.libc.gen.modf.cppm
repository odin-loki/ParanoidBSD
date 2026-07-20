module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.modf;

export import pbsd.core;

/// modf from hbsd/src/lib/libc/gen/modf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double modf_frac(double x, double* iptr) noexcept { if (iptr) *iptr = x; return 0.0; }

} // namespace pbsd::userland::libc
