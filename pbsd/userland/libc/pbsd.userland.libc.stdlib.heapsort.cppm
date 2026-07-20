module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.heapsort;

/// heapsort from hbsd/src/lib/libc/stdlib/heapsort.c (sift-down subset)
export namespace pbsd::userland::libc {

template <typename Compare>
inline void heapsort(void* base, std::size_t nmemb, std::size_t size, Compare compar) {
    if (nmemb < 2 || size == 0) {
        return;
    }
    auto* a = static_cast<char*>(base);

    auto sift_down = [&](std::size_t root, std::size_t count) {
        for (std::size_t i = root;;) {
            std::size_t best = i;
            const std::size_t left = 2 * i + 1;
            const std::size_t right = left + 1;
            if (left < count &&
                compar(a + left * size, a + best * size) > 0) {
                best = left;
            }
            if (right < count &&
                compar(a + right * size, a + best * size) > 0) {
                best = right;
            }
            if (best == i) {
                break;
            }
            for (std::size_t b = 0; b < size; ++b) {
                const char t = a[i * size + b];
                a[i * size + b] = a[best * size + b];
                a[best * size + b] = t;
            }
            i = best;
        }
    };

    for (std::size_t i = nmemb / 2; i-- > 0;) {
        sift_down(i, nmemb);
    }
    for (std::size_t end = nmemb; end > 1; --end) {
        for (std::size_t b = 0; b < size; ++b) {
            const char t = a[b];
            a[b] = a[(end - 1) * size + b];
            a[(end - 1) * size + b] = t;
        }
        sift_down(0, end - 1);
    }
}

} // namespace pbsd::userland::libc
