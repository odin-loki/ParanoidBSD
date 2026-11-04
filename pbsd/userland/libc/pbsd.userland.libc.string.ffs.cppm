module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.ffs;

export import pbsd.core;

/// ffs from hbsd/src/lib/libc/string/ffs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ffs_val(int i) noexcept {
    if (i == 0) return 0;
    int n = 1;
    while ((i & 1) == 0) { i >>= 1; ++n; }
    return n;
}

} // namespace pbsd::userland::libc
