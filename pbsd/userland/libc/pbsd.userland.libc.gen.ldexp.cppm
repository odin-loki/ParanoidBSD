module;

#include <cmath>

export module pbsd.userland.libc.gen.ldexp;

/// ldexp from hbsd/src/lib/libc/gen/ldexp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double ldexp(double x, int exp) noexcept { return std::ldexp(x, exp); }

[[nodiscard]] inline float ldexpf(float x, int exp) noexcept { return std::ldexp(x, exp); }

} // namespace pbsd::userland::libc
