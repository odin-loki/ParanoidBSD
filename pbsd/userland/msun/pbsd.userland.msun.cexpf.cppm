module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cexpf;

/// cexpf from hbsd/src/lib/msun/src/s_cexpf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cexpf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cexpff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
