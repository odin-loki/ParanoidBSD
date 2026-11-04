module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.flsll;

export import pbsd.core;

/// flsll from hbsd/src/lib/libc/string/flsll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int flsll_val(long long i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned long long u = static_cast<unsigned long long>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
