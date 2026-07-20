module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.lgammaf_r;

/// lgammaf_r from hbsd/src/lib/msun/src/e_lgammaf_r.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double lgammaf_r_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float lgammaf_rf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
