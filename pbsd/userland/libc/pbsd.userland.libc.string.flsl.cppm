module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.flsl;

export import pbsd.core;

/// flsl from hbsd/src/lib/libc/string/flsl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int flsl_val(long i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned long u = static_cast<unsigned long>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
