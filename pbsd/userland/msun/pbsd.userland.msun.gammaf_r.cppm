module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.gammaf_r;

/// gammaf_r from hbsd/src/lib/msun/src/w_gammaf_r.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double gammaf_r_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float gammaf_rf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
