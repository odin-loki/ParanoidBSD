module;

#include <cmath>

export module pbsd.userland.msun.remquo;

/// remquo/remquof from hbsd/src/lib/msun/src/s_remquo.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double remquo(double x, double y, int* quo) noexcept {
    return std::remquo(x, y, quo);
}

[[nodiscard]] inline float remquof(float x, float y, int* quo) noexcept {
    return std::remquof(x, y, quo);
}

} // namespace pbsd::userland::msun
