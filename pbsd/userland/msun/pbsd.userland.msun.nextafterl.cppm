module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.nextafterl;

/// nextafterl from hbsd/src/lib/msun/src/s_nextafterl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double nextafterl(long double x, long double y) noexcept {
    if (x == y) {
        return y;
    }
    std::uint64_t ux;
    std::memcpy(&ux, &x, sizeof(ux));
    if (ux == 0) {
        std::uint64_t hy;
        std::memcpy(&hy, &y, sizeof(hy));
        ux = (hy & (std::uint64_t(1) << 63)) | 1;
        std::memcpy(&x, &ux, sizeof(x));
        return x;
    }
    const bool upward = (x < y) ^ ((ux >> 63) != 0);
    if (upward) {
        ++ux;
    } else {
        --ux;
    }
    std::memcpy(&x, &ux, sizeof(x));
    return x;
}

} // namespace pbsd::userland::msun
