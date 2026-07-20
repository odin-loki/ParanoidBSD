module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_numf;

/// fminimum_numf from hbsd/src/lib/msun/src/s_fminimum_numf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fminimum_numf(float x, float y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
