module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.libc.string.swab;

/// swab from hbsd/src/lib/libc/string/swab.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint16_t bswap16(std::uint16_t x) noexcept {
    return static_cast<std::uint16_t>((x >> 8) | (x << 8));
}

inline void swab(const void* from, void* to, long len) noexcept {
    const auto* f = static_cast<const char*>(from);
    auto* t = static_cast<char*>(to);
    std::uint16_t tmp;

    while (len > 1) {
        std::memcpy(&tmp, f, 2);
        tmp = bswap16(tmp);
        std::memcpy(t, &tmp, 2);
        f += 2;
        t += 2;
        len -= 2;
    }
}

} // namespace pbsd::userland::libc
