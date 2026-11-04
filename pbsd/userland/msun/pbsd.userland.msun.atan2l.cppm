module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atan2l;

/// atan2l from hbsd/src/lib/msun/src/e_atan2l.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double atan2l_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float atan2lf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
