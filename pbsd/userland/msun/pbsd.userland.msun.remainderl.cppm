module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.remainderl;

/// remainderl from hbsd/src/lib/msun/src/e_remainderl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double remainderl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float remainderlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
