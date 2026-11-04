module;
#include <cstddef>

export module pbsd.userland.libc.stdlib.qsort;

export import pbsd.core;

/// qsort from hbsd/src/lib/libc/stdlib/qsort.c
export namespace pbsd::userland::libc {

using QsortCompare = int (*)(const void*, const void*) noexcept;

inline void qsort_swap(unsigned char* a, unsigned char* b, std::size_t size) noexcept {
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned char t = a[i];
        a[i] = b[i];
        b[i] = t;
    }
}

} // namespace pbsd::userland::libc
