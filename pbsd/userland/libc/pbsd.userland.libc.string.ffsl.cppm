module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.ffsl;

export import pbsd.core;

/// ffsl from hbsd/src/lib/libc/string/ffsl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ffsl_val(long i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1L) == 0) { i >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
