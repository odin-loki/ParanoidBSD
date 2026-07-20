module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.ffsll;

export import pbsd.core;

/// ffsll from hbsd/src/lib/libc/string/ffsll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ffsll_val(long long i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1LL) == 0) { i >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
