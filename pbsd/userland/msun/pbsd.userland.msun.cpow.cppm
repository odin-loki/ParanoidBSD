module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cpow;

/// cpow from hbsd/src/lib/msun/src/s_cpow.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cpow_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cpowf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
