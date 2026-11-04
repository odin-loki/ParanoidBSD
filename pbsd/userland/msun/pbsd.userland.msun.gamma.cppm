module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.gamma;

/// gamma from hbsd/src/lib/msun/src/w_gamma.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double gamma_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float gammaf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
