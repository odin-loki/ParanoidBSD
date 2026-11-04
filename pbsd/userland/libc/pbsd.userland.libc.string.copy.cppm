module;

#include <cstddef>

export module pbsd.userland.libc.string.copy;

/// strcpy/strncpy/strcat/strlcpy/strnlen from hbsd/src/lib/libc/string/*.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strcpy(char* dst, const char* src) noexcept {
    char* save = dst;
    while ((*dst = *src) != '\0') {
        ++src;
        ++dst;
    }
    return save;
}

[[nodiscard]] inline char* strncpy(char* dst, const char* src, std::size_t n) noexcept {
    if (n != 0) {
        char* d = dst;
        const char* s = src;
        do {
            if ((*d++ = *s++) == '\0') {
                while (--n != 0) {
                    *d++ = '\0';
                }
                break;
            }
        } while (--n != 0);
    }
    return dst;
}

[[nodiscard]] inline char* strcat(char* s, const char* append) noexcept {
    char* save = s;
    while (*s != '\0') {
        ++s;
    }
    while ((*s++ = *append++) != '\0') {
    }
    return save;
}

[[nodiscard]] inline std::size_t strlcpy(char* dst, const char* src,
                                           std::size_t dsize) noexcept {
    const char* osrc = src;
    std::size_t nleft = dsize;

    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') {
                break;
            }
        }
    }

    if (nleft == 0) {
        if (dsize != 0) {
            *dst = '\0';
        }
        while (*src++ != '\0') {
        }
    }

    return static_cast<std::size_t>(src - osrc - 1);
}

[[nodiscard]] inline std::size_t strnlen(const char* s, std::size_t maxlen) noexcept {
    std::size_t len = 0;
    for (; len < maxlen; ++len, ++s) {
        if (*s == '\0') {
            break;
        }
    }
    return len;
}

} // namespace pbsd::userland::libc
