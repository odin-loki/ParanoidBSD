module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimum_magf;

/// fminimum_magf from hbsd/src/lib/msun/src/s_fminimum_magf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fminimum_magf(float x, float y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
