module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_mag_numf;

/// fminimum_mag_numf from hbsd/src/lib/msun/src/s_fminimum_mag_numf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fminimum_mag_numf(float x, float y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
