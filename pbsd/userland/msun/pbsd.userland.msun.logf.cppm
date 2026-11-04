module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.logf;

/// logf from hbsd/src/lib/msun/src/e_logf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double logf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float logff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
