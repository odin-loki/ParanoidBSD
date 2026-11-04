module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.asinl;

/// asinl from hbsd/src/lib/msun/src/e_asinl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double asinl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float asinlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
