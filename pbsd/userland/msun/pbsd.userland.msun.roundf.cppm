module;

#include <cmath>

export module pbsd.userland.msun.roundf;

/// roundf from hbsd/src/lib/msun/src/s_roundf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float roundf(float x) noexcept { return std::round(x); }

} // namespace pbsd::userland::msun
