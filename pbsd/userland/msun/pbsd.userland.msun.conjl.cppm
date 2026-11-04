module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.conjl;

/// conjl from hbsd/src/lib/msun/src/s_conjl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double conjl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float conjlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
