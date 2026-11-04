module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.ceilf;

/// ceilf from hbsd/src/lib/msun/src/s_ceilf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float ceilf(float x) noexcept {
    std::uint32_t i0;
    std::memcpy(&i0, &x, sizeof(i0));
    const int j0 = static_cast<int>(((i0 >> 23) & 0xffU) - 0x7fU);
    if (j0 < 23) {
        if (j0 < 0) {
            if ((i0 & 0x7fffffffU) != 0U) {
                i0 = (i0 & 0x80000000U) ? 0x80000000U : 0x3f800000U;
            }
        } else {
            const std::uint32_t i = 0x007fffffU >> j0;
            if ((i0 & i) == 0U) {
                return x;
            }
            if (i0 < 0x80000000U) {
                i0 += 0x00800000U >> j0;
            }
            i0 &= ~i;
        }
    } else if (j0 == 0x80) {
        return x + x;
    }
    std::memcpy(&x, &i0, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
