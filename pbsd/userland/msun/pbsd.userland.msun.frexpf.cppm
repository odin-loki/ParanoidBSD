module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.frexpf;

/// frexpf from hbsd/src/lib/msun/src/s_frexpf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float frexpf_val(float x, int* exp) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int e = static_cast<int>((u >> 23) & 0xff) - 126;
    if (((u >> 23) & 0xff) == 0) {
        if (exp != nullptr) {
            *exp = 0;
        }
        return x;
    }
    u &= 0x807fffffu;
    u |= static_cast<std::uint32_t>(126) << 23;
    std::memcpy(&x, &u, sizeof(u));
    if (exp != nullptr) {
        *exp = e;
    }
    return x;
}

} // namespace pbsd::userland::msun
