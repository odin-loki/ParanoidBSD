module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cosf;

/// cosf from hbsd/src/lib/msun/src/e_cosf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float cosf_val(float x) noexcept { return ::cosf(x); }

} // namespace pbsd::userland::msun
