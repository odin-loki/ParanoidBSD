module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.carg;

/// carg from hbsd/src/lib/msun/src/s_carg.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double carg_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cargf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
