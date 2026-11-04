module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cargf;

/// cargf from hbsd/src/lib/msun/src/s_cargf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cargf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cargff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
