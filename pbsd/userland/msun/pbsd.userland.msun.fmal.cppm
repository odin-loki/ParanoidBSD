module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmal;

/// fmal from hbsd/src/lib/msun/src/s_fmal.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmal_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float fmalf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
