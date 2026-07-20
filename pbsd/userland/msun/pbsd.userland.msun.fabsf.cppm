module;
#include <cmath>
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fabsf;

/// fabsf from hbsd/src/lib/msun/src/s_fabsf.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline float fabsf_val(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    u &= 0x7fffffffu;
    std::memcpy(&x, &u, sizeof(u));
    return x;
}

} // namespace pbsd::userland::msun
