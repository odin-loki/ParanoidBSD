module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminimumf;

/// fminimumf from hbsd/src/lib/msun/src/s_fminimumf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fminimumf(float x, float y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
