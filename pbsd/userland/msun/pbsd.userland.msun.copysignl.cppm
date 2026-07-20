module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.copysignl;

/// copysignl from hbsd/src/lib/msun/src/s_copysignl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double copysignl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float copysignlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
