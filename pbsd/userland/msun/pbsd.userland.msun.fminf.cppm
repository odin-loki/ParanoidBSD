module;
#include <cmath>

export module pbsd.userland.msun.fminf;

/// fminf from hbsd/src/lib/msun/src/s_fminf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fminf_val(float x, float y) noexcept { return std::fminf(x, y); }

} // namespace pbsd::userland::msun
