module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.coshf;

/// coshf from hbsd/src/lib/msun/src/e_coshf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float coshf_val(float x) noexcept { return ::coshf(x); }

} // namespace pbsd::userland::msun
