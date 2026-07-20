module;

export module pbsd.userland.msun.fmod;

/// fmod/fmodf from hbsd/src/lib/msun/src/e_fmod.c (shift-and-subtract stub)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fmod(double x, double y) noexcept {
    if (y == 0.0) {
        return 0.0 / 0.0;
    }
    if (x < 0.0) {
        return -fmod(-x, y);
    }
    if (y < 0.0) {
        y = -y;
    }
    while (x >= y) {
        x -= y;
    }
    return x;
}

[[nodiscard]] inline float fmodf(float x, float y) noexcept {
    return static_cast<float>(fmod(static_cast<double>(x), static_cast<double>(y)));
}

} // namespace pbsd::userland::msun
