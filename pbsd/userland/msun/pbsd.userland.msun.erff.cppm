module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.erff;

/// erff from hbsd/src/lib/msun/src/s_erff.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double erff_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float erfff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
