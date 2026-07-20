module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_mag_numf;

/// fmaximum_mag_numf from hbsd/src/lib/msun/src/s_fmaximum_mag_numf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmaximum_mag_numf(float x, float y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
