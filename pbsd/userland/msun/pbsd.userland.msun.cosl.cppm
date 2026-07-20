module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cosl;

/// cosl from hbsd/src/lib/msun/src/e_cosl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cosl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float coslf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
