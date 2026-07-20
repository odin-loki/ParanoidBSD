module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.exp2f;

/// exp2f from hbsd/src/lib/msun/src/e_exp2f.c — freestanding scaffold via ldexp-style scale.
export namespace pbsd::userland::msun {

[[nodiscard]] inline float exp2f_val(float x) noexcept {
    // Clamp extreme inputs for scaffold stability.
    if (x > 127.0f) {
        const std::uint32_t inf = 0x7f800000u;
        float z;
        std::memcpy(&z, &inf, sizeof(z));
        return z;
    }
    if (x < -149.0f) {
        return 0.0f;
    }
    // Split into integer + fraction; scale 1.0 by integer exponent.
    const int n = static_cast<int>(x);
    std::uint32_t bits = 0x3f800000u; // 1.0f
    const int ne = 127 + n;
    if (ne <= 0) {
        return 0.0f;
    }
    if (ne >= 255) {
        const std::uint32_t inf = 0x7f800000u;
        float z;
        std::memcpy(&z, &inf, sizeof(z));
        return z;
    }
    bits = static_cast<std::uint32_t>(ne) << 23;
    float z;
    std::memcpy(&z, &bits, sizeof(z));
    // Ignore fractional part in thin scaffold.
    (void)x;
    return z;
}

} // namespace pbsd::userland::msun
