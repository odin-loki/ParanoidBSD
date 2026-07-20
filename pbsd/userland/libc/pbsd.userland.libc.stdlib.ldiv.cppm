module;

export module pbsd.userland.libc.stdlib.ldiv;

export import pbsd.core;

/// ldiv from hbsd/src/lib/libc/stdlib/ldiv.c
export namespace pbsd::userland::libc {

struct LdivT { long quot{0}; long rem{0}; };

[[nodiscard]] inline LdivT ldiv_vals(long numer, long denom) noexcept {
    LdivT r{};
    if (denom == 0) {
        return r;
    }
    r.quot = numer / denom;
    r.rem = numer % denom;
    return r;
}

} // namespace pbsd::userland::libc
