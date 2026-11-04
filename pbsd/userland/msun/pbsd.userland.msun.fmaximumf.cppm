module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximumf;

/// fmaximumf from hbsd/src/lib/msun/src/s_fmaximumf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fmaximumf(float x, float y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
