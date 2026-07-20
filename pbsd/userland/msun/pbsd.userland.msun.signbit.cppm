module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.signbit;

/// signbit from hbsd/src/lib/msun/src/s_signbit.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline bool signbit(double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u >> 63) != 0;
}

[[nodiscard]] inline bool signbitf(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u >> 31) != 0;
}

} // namespace pbsd::userland::msun
