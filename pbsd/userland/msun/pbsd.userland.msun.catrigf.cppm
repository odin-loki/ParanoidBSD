module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.catrigf;

/// catrigf from hbsd/src/lib/msun/src/s_catrigf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double catrigf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float catrigff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
