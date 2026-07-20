module;

#include <cstdint>

export module pbsd.userland.libc.stdlib.imaxdiv;

/// imaxdiv from hbsd/src/lib/libc/stdlib/imaxdiv.c
export namespace pbsd::userland::libc {

struct imaxdiv_t {
    std::intmax_t quot;
    std::intmax_t rem;
};

[[nodiscard]] inline imaxdiv_t imaxdiv(std::intmax_t numer, std::intmax_t denom) noexcept {
    imaxdiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
