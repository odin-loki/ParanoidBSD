module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.coshl;

/// coshl from hbsd/src/lib/msun/src/e_coshl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double coshl_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float coshlf_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
