module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cargl;

/// cargl from hbsd/src/lib/msun/src/s_cargl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cargl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float carglf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
