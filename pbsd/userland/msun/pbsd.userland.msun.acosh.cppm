module;
#include <cmath>

export module pbsd.userland.msun.acosh;

/// acosh from hbsd/src/lib/msun/src/e_acosh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double acosh(double x) noexcept {
    return std::acosh(x);
}

[[nodiscard]] inline float acoshf(float x) noexcept {
    return std::acoshf(x);
}

} // namespace pbsd::userland::msun
