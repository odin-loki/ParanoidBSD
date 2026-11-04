module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cimagl;

/// cimagl from hbsd/src/lib/msun/src/s_cimagl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cimagl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cimaglf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
