module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fabsl;

/// fabsl from hbsd/src/lib/msun/src/s_fabsl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fabsl_val(long double x) noexcept {
    std::uint64_t u;
    std::memcpy(&u, &x, sizeof(u));
    u &= 0x7fffffffffffffffULL;
    std::memcpy(&x, &u, sizeof(u));
    return x;
}

} // namespace pbsd::userland::msun
