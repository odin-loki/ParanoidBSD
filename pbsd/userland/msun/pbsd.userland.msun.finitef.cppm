module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.finitef;

/// finitef from hbsd/src/lib/msun/src/s_finitef.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline int finitef_val(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    return (u & 0x7fffffffu) != 0x7f800000u;
}

} // namespace pbsd::userland::msun
