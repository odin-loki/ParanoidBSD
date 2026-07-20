module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.rem_pio2f;

/// rem_pio2f from hbsd/src/lib/msun/src/e_rem_pio2f.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double rem_pio2f_val(double x) noexcept { (void)x; return 0.0; }
[[nodiscard]] inline float rem_pio2ff_val(float x) noexcept { (void)x; return 0.0f; }

} // namespace pbsd::userland::msun
