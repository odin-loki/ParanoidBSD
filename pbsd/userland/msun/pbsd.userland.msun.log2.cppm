module;
#include <cmath>

export module pbsd.userland.msun.log2;

/// log2 from hbsd/src/lib/msun/src/e_log2.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double log2(double x) noexcept {
    return std::log2(x);
}

[[nodiscard]] inline float log2f(float x) noexcept {
    return std::log2f(x);
}

} // namespace pbsd::userland::msun
