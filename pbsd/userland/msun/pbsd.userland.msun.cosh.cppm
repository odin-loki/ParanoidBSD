module;

export module pbsd.userland.msun.cosh;

import pbsd.userland.msun.exp;

/// cosh/coshf from hbsd/src/lib/msun/src/s_cosh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cosh(double x) noexcept {
    return (exp(x) + exp(-x)) / 2.0;
}

[[nodiscard]] inline float coshf(float x) noexcept {
    return static_cast<float>(cosh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
