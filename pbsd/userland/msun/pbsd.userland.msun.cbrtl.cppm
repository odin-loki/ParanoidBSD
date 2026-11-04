module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cbrtl;

/// cbrtl from hbsd/src/lib/msun/src/s_cbrtl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cbrtl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cbrtlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
