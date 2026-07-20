module;

#include <cmath>

export module pbsd.userland.msun.lrint;

/// lrint/lrintf from hbsd/src/lib/msun/src/s_lrintf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long lrint(double x) noexcept { return std::lrint(x); }

[[nodiscard]] inline long lrintf(float x) noexcept { return std::lrint(x); }

} // namespace pbsd::userland::msun
