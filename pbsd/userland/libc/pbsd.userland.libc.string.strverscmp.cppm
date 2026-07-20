module;

#include <cstddef>

export module pbsd.userland.libc.string.strverscmp;

/// strverscmp from hbsd/src/lib/libc/string/strverscmp.c (version sort subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strverscmp(const char* s1, const char* s2) noexcept {
    while (*s1 != '\0' && *s2 != '\0') {
        if (*s1 != *s2) {
            const bool d1 = *s1 >= '0' && *s1 <= '9';
            const bool d2 = *s2 >= '0' && *s2 <= '9';
            if (!d1 || !d2) {
                return static_cast<unsigned char>(*s1) < static_cast<unsigned char>(*s2) ? -1 : 1;
            }
            while (*s1 == '0') {
                ++s1;
            }
            while (*s2 == '0') {
                ++s2;
            }
            std::size_t z1 = 0;
            std::size_t z2 = 0;
            while (s1[z1] >= '0' && s1[z1] <= '9') {
                ++z1;
            }
            while (s2[z2] >= '0' && s2[z2] <= '9') {
                ++z2;
            }
            if (z1 != z2) {
                return z1 < z2 ? -1 : 1;
            }
            while (z1 != 0) {
                if (*s1 != *s2) {
                    return *s1 < *s2 ? -1 : 1;
                }
                ++s1;
                ++s2;
                --z1;
            }
            continue;
        }
        ++s1;
        ++s2;
    }
    if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    }
    if (*s1 == '\0') {
        return -1;
    }
    return 1;
}

} // namespace pbsd::userland::libc
