module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.csqrt;

/// csqrt from hbsd/src/lib/msun/src/s_csqrt.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double csqrt_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float csqrtf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
