module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.atanhl;

/// atanhl from hbsd/src/lib/msun/src/e_atanhl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double atanhl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float atanhlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
