module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cexp;

/// cexp from hbsd/src/lib/msun/src/s_cexp.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cexp_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cexpf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
