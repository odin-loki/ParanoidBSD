module;
#include <cmath>

export module pbsd.userland.msun.llroundf;

/// llroundf from hbsd/src/lib/msun/src/s_llroundf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long long llroundf(float x) noexcept {
    return std::llround(x);
}

} // namespace pbsd::userland::msun
