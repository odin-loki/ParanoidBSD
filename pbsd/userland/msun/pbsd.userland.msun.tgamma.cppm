module;
#include <cmath>

export module pbsd.userland.msun.tgamma;

/// tgamma from hbsd/src/lib/msun/src/e_tgamma.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tgamma(double x) noexcept {
    return std::tgamma(x);
}

[[nodiscard]] inline float tgammaf(float x) noexcept {
    return std::tgammaf(x);
}

} // namespace pbsd::userland::msun
