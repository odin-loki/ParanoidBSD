module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_numf;

/// fmaximum_numf from hbsd/src/lib/msun/src/s_fmaximum_numf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmaximum_numf(float x, float y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
