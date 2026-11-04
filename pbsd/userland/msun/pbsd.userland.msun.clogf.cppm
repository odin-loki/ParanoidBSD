module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.clogf;

/// clogf from hbsd/src/lib/msun/src/s_clogf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double clogf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float clogff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
