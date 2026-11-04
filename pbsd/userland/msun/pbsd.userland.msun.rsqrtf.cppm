module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.rsqrtf;

/// rsqrtf from hbsd/src/lib/msun/src/s_rsqrtf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float rsqrtf_val(float x) noexcept { return x; }

} // namespace pbsd::userland::msun
