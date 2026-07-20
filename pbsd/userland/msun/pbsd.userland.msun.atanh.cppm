module;
#include <cmath>

export module pbsd.userland.msun.atanh;

/// atanh from hbsd/src/lib/msun/src/e_atanh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double atanh(double x) noexcept {
    return std::atanh(x);
}

[[nodiscard]] inline float atanhf(float x) noexcept {
    return std::atanhf(x);
}

} // namespace pbsd::userland::msun
