module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.csqrtf;

/// csqrtf from hbsd/src/lib/msun/src/s_csqrtf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double csqrtf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float csqrtff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
