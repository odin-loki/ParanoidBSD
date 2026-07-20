module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.hypotl;

/// hypotl from hbsd/src/lib/msun/src/e_hypotl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double hypotl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float hypotlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
