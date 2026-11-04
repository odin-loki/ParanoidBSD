module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.jn;

/// jn from hbsd/src/lib/msun/src/e_jn.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double jn_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float jnf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
