module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cpowl;

/// cpowl from hbsd/src/lib/msun/src/s_cpowl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cpowl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cpowlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
