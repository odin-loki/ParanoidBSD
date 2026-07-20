module;

export module pbsd.userland.msun.tanh;

import pbsd.userland.msun.sinh;
import pbsd.userland.msun.cosh;

/// tanh/tanhf from hbsd/src/lib/msun/src/s_tanh.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double tanh(double x) noexcept {
    const double c = cosh(x);
    if (c == 0.0) {
        return 0.0;
    }
    return sinh(x) / c;
}

[[nodiscard]] inline float tanhf(float x) noexcept {
    return static_cast<float>(tanh(static_cast<double>(x)));
}

} // namespace pbsd::userland::msun
