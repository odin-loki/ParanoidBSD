module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximum_magf;

/// fmaximum_magf from hbsd/src/lib/msun/src/s_fmaximum_magf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmaximum_magf(float x, float y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
