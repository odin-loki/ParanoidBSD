module;

#include <cmath>

export module pbsd.userland.msun.erfc;

/// erfc from hbsd/src/lib/msun/src/s_erfc.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double erfc(double x) noexcept { return std::erfc(x); }

[[nodiscard]] inline float erfcf(float x) noexcept { return std::erfcf(x); }

} // namespace pbsd::userland::msun
