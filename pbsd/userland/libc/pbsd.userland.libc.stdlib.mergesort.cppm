module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.mergesort;

/// mergesort merge step from hbsd/src/lib/libc/stdlib/merge.c
export namespace pbsd::userland::libc {

template <typename Compare>
inline void merge_ranges(char* base, std::size_t size, std::size_t mid, std::size_t nmemb,
                         Compare compar, char* tmp) {
    std::size_t i = 0;
    std::size_t j = mid;
    std::size_t k = 0;
    while (i < mid && j < nmemb) {
        const char* left = base + i * size;
        const char* right = base + j * size;
        if (compar(left, right) <= 0) {
            for (std::size_t b = 0; b < size; ++b) {
                tmp[k * size + b] = left[b];
            }
            ++i;
        } else {
            for (std::size_t b = 0; b < size; ++b) {
                tmp[k * size + b] = right[b];
            }
            ++j;
        }
        ++k;
    }
    while (i < mid) {
        for (std::size_t b = 0; b < size; ++b) {
            tmp[k * size + b] = base[i * size + b];
        }
        ++i;
        ++k;
    }
    while (j < nmemb) {
        for (std::size_t b = 0; b < size; ++b) {
            tmp[k * size + b] = base[j * size + b];
        }
        ++j;
        ++k;
    }
    for (std::size_t t = 0; t < k * size; ++t) {
        base[t] = tmp[t];
    }
}

} // namespace pbsd::userland::libc
