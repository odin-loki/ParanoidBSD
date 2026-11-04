module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaf;

/// fmaf from hbsd/src/lib/msun/src/s_fmaf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmaf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float fmaff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
