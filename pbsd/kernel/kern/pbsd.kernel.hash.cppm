module;
#include <cstdint>

export module pbsd.kernel.hash;

export import pbsd.core;

/// Freestanding port of `sys/hash.h` / `kern/subr_hash.c`.
export namespace pbsd::kernel::hash {

inline constexpr std::uint32_t kInit = 5381;

[[nodiscard]] constexpr std::uint32_t step(std::uint32_t h, unsigned char c) noexcept {
    return (h << 5) + h + static_cast<std::uint32_t>(c);
}

[[nodiscard]] inline std::uint32_t buf32(const unsigned char* p, std::size_t len,
                                         std::uint32_t seed = kInit) noexcept {
    if (p == nullptr) {
        return seed;
    }
    std::uint32_t h = seed;
    while (len-- > 0) {
        h = step(h, *p++);
    }
    return h;
}

[[nodiscard]] inline std::uint32_t str32(const char* s, std::uint32_t seed = kInit) noexcept {
    if (s == nullptr) {
        return seed;
    }
    std::uint32_t h = seed;
    while (*s != '\0') {
        h = step(h, static_cast<unsigned char>(*s++));
    }
    return h;
}

} // namespace pbsd::kernel::hash
