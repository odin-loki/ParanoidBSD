module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.jnf;

/// jnf from hbsd/src/lib/msun/src/e_jnf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double jnf_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float jnff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
