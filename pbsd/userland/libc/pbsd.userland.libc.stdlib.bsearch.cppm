module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.bsearch;

/// bsearch from hbsd/src/lib/libc/stdlib/bsearch.c
export namespace pbsd::userland::libc {

using CompareFn = int (*)(const void*, const void*) noexcept;

[[nodiscard]] inline void* bsearch(const void* key, const void* base, std::size_t nmemb,
                                   std::size_t size, CompareFn compar) noexcept {
    if (key == nullptr || base == nullptr || compar == nullptr || nmemb == 0 || size == 0) {
        return nullptr;
    }
    const auto* table = static_cast<const unsigned char*>(base);
    std::size_t lim = nmemb;
    while (lim != 0) {
        const std::size_t half = lim / 2;
        const auto* pivot = table + half * size;
        const int cmp = compar(key, pivot);
        if (cmp == 0) {
            return const_cast<void*>(static_cast<const void*>(pivot));
        }
        if (cmp > 0) {
            table = pivot + size;
            lim -= half + 1;
        } else {
            lim = half;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
