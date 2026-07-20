module;

#include <cstddef>

export module pbsd.userland.libc.string.concat;

import pbsd.userland.libc.string;

/// strncat/strlcat from hbsd/src/lib/libc/string/{strncat,strlcat}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strncat(char* dst, const char* src, std::size_t n) noexcept {
    if (n != 0) {
        char* d = dst;
        const char* s = src;
        while (*d != '\0') {
            ++d;
        }
        do {
            if ((*d = *s++) == '\0') {
                break;
            }
            ++d;
        } while (--n != 0);
        *d = '\0';
    }
    return dst;
}

[[nodiscard]] inline std::size_t strlcat(char* dst, const char* src, std::size_t dsize) noexcept {
    const char* odst = dst;
    const char* osrc = src;
    std::size_t n = dsize;
    std::size_t dlen;

    while (n-- != 0 && *dst != '\0') {
        ++dst;
    }
    dlen = static_cast<std::size_t>(dst - odst);
    n = dsize - dlen;

    if (n-- == 0) {
        return dlen + strlen(src);
    }
    while (*src != '\0') {
        if (n != 0) {
            *dst++ = *src;
            --n;
        }
        ++src;
    }
    *dst = '\0';
    return dlen + static_cast<std::size_t>(src - osrc);
}

} // namespace pbsd::userland::libc
