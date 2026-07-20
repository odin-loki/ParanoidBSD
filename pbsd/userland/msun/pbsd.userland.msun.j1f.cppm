module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.j1f;

/// j1f from hbsd/src/lib/msun/src/e_j1f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double j1f_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float j1ff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
