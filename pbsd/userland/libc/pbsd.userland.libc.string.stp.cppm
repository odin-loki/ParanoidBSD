module;

#include <cstddef>

export module pbsd.userland.libc.string.stp;

/// stpcpy/stpncpy from hbsd/src/lib/libc/string/{stpcpy,stpncpy}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* stpcpy(char* to, const char* from) noexcept {
    for (; (*to = *from) != '\0'; ++from, ++to) {
    }
    return to;
}

[[nodiscard]] inline char* stpncpy(char* dst, const char* src, std::size_t n) noexcept {
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

} // namespace pbsd::userland::libc
