module;
#include <cmath>

export module pbsd.userland.msun.fmaxf;

/// fmaxf from hbsd/src/lib/msun/src/s_fmaxf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmaxf_val(float x, float y) noexcept { return std::fmaxf(x, y); }

} // namespace pbsd::userland::msun
