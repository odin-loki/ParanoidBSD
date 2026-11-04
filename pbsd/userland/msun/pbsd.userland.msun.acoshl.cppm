module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.acoshl;

/// acoshl from hbsd/src/lib/msun/src/e_acoshl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double acoshl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float acoshlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
