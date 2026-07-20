module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cimag;

/// cimag from hbsd/src/lib/msun/src/s_cimag.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cimag_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cimagf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
