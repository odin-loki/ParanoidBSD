module;

export module pbsd.userland.msun.pow;

import pbsd.userland.msun.exp;
import pbsd.userland.msun.log;

/// pow/powf from hbsd/src/lib/msun/src/e_pow.c (exp(y*log(x)))
export namespace pbsd::userland::msun {

[[nodiscard]] inline double pow(double x, double y) noexcept {
    if (x == 0.0) {
        return y > 0.0 ? 0.0 : x / 0.0;
    }
    if (x < 0.0) {
        return x / 0.0;
    }
    return exp(y * log(x));
}

[[nodiscard]] inline float powf(float x, float y) noexcept {
    return static_cast<float>(pow(static_cast<double>(x), static_cast<double>(y)));
}

} // namespace pbsd::userland::msun
