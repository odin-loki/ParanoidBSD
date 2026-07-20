module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cprojl;

/// cprojl from hbsd/src/lib/msun/src/s_cprojl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cprojl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cprojlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
