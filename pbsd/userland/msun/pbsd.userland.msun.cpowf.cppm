module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cpowf;

/// cpowf from hbsd/src/lib/msun/src/s_cpowf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cpowf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float cpowff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
