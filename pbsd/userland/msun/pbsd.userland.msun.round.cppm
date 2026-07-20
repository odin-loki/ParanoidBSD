module;

#include <cmath>

export module pbsd.userland.msun.round;

/// round/roundf from hbsd/src/lib/msun/src/s_round.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double round(double x) noexcept { return std::round(x); }

} // namespace pbsd::userland::msun
