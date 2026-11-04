module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.csqrtl;

/// csqrtl from hbsd/src/lib/msun/src/s_csqrtl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double csqrtl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float csqrtlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
