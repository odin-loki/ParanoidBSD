module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.asinhl;

/// asinhl from hbsd/src/lib/msun/src/e_asinhl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double asinhl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float asinhlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
