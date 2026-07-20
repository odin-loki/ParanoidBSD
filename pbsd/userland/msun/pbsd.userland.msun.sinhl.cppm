module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.sinhl;

/// sinhl from hbsd/src/lib/msun/src/e_sinhl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double sinhl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float sinhlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
