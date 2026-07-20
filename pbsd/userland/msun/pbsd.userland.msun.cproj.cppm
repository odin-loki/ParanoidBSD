module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cproj;

/// cproj from hbsd/src/lib/msun/src/s_cproj.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cproj_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cprojf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
