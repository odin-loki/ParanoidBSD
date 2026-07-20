module;

#include <cstdlib>

export module pbsd.userland.libc.stdlib.lldiv;

/// lldiv from hbsd/src/lib/libc/stdlib/lldiv.c
export namespace pbsd::userland::libc {

struct lldiv_t {
    long long quot;
    long long rem;
};

[[nodiscard]] inline lldiv_t lldiv(long long numer, long long denom) noexcept {
    lldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
