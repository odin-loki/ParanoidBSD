module;

#include <cstdlib>

export module pbsd.userland.libc.stdlib.div;

/// div/ldiv/lldiv from hbsd/src/lib/libc/stdlib/{div,ldiv,lldiv}.c
export namespace pbsd::userland::libc {

struct lldiv_t {
    long long quot;
    long long rem;
};

[[nodiscard]] inline div_t div(int numer, int denom) noexcept {
    div_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

[[nodiscard]] inline ldiv_t ldiv(long numer, long denom) noexcept {
    ldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

[[nodiscard]] inline lldiv_t lldiv(long long numer, long long denom) noexcept {
    lldiv_t r;
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
