module;

export module pbsd.userland.msun.cos;

import pbsd.userland.msun.sin;

/// cos/cosf from hbsd/src/lib/msun/src/{k_cos,s_cos}.c (phase-shifted sin)
export namespace pbsd::userland::msun {

namespace detail {
inline constexpr double kHalfPi = 1.57079632679489661923;
} // namespace detail

[[nodiscard]] inline double cos(double x) noexcept {
    return sin(x + detail::kHalfPi);
}

[[nodiscard]] inline float cosf(float x) noexcept {
    return static_cast<float>(cos(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
