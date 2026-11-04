module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.libc.gdtoa.ldisf;

/// machdep_ldisf from hbsd/src/lib/libc/gdtoa/machdep_ldisf.c (float exponent)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ldisf(float x) noexcept {
    std::uint32_t u;
    std::memcpy(&u, &x, sizeof(u));
    const int exp = static_cast<int>((u >> 23) & 0xff);
    if (exp == 0) {
        return 1 - 126;
    }
    if (exp == 0xff) {
        return 0;
    }
    return exp - 127;
}

} // namespace pbsd::userland::libc
