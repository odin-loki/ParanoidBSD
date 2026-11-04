module;
#include <cstdint>

export module pbsd.userland.libc.gen.arc4random_uniform;

/// arc4random_uniform from hbsd/src/lib/libc/gen/arc4random_uniform.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t arc4random_uniform(std::uint32_t upper_bound,
                                                     std::uint32_t raw) noexcept {
    if (upper_bound < 2) {
        return 0;
    }
    const std::uint32_t min = static_cast<std::uint32_t>(-upper_bound) % upper_bound;
    std::uint32_t r = raw;
    while (r < min) {
        r = raw;
    }
    return r % upper_bound;
}

} // namespace pbsd::userland::libc
