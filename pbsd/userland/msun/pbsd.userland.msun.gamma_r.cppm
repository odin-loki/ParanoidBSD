module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.gamma_r;

/// gamma_r from hbsd/src/lib/msun/src/w_gamma_r.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double gamma_r_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float gamma_rf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
