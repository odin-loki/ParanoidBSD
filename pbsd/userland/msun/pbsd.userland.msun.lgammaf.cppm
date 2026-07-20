module;
#include <cmath>

export module pbsd.userland.msun.lgammaf;

/// lgammaf from hbsd/src/lib/msun/src/e_lgammaf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float lgammaf_val(float x) noexcept { return std::lgammaf(x); }

} // namespace pbsd::userland::msun
