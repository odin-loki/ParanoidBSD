module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.acosl;

/// acosl from hbsd/src/lib/msun/src/e_acosl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double acosl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float acoslf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
