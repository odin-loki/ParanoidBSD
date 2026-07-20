module;

#include <cstdint>

export module pbsd.userland.libc.stdlib.imax;

/// imaxabs/imaximin/imaxmax from hbsd/src/lib/libc/stdlib/imaxabs.c + stdint helpers
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::intmax_t imaxabs(std::intmax_t j) noexcept {
    return j < 0 ? -j : j;
}

[[nodiscard]] inline std::intmax_t imaxmin(std::intmax_t a, std::intmax_t b) noexcept {
    return a < b ? a : b;
}

[[nodiscard]] inline std::intmax_t imaxmax(std::intmax_t a, std::intmax_t b) noexcept {
    return a > b ? a : b;
}

} // namespace pbsd::userland::libc
