module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.fls;

export import pbsd.core;

/// fls from hbsd/src/lib/libc/string/fls.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int fls_val(int i) noexcept {
    if (i == 0) return 0;
    int n = 0;
    unsigned u = static_cast<unsigned>(i);
    while (u) { u >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
