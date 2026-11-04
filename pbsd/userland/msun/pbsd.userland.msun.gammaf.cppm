module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.gammaf;

/// gammaf from hbsd/src/lib/msun/src/w_gammaf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double gammaf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float gammaff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
