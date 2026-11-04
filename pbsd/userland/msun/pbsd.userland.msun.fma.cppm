module;

export module pbsd.userland.msun.fma;

/// fma/fmaf from hbsd/src/lib/msun/src/s_fma.c (fused multiply-add stub)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double fma(double x, double y, double z) noexcept { return x * y + z; }

[[nodiscard]] inline float fmaf(float x, float y, float z) noexcept {
    return static_cast<float>(fma(static_cast<double>(x), static_cast<double>(y),
                                   static_cast<double>(z)));
}

} // namespace pbsd::userland::msun
