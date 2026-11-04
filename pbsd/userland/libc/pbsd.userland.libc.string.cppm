module;

#include <climits>
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string;

/// Freestanding string helpers from hbsd/src/lib/libc/string/{strlen,strcmp,strncmp}.c
export namespace pbsd::userland::libc {

#if ULONG_MAX == 0xffffffffffffffffULL
inline constexpr unsigned long kStrlenMask01 = 0x0101010101010101UL;
inline constexpr unsigned long kStrlenMask80 = 0x8080808080808080UL;
#else
inline constexpr unsigned long kStrlenMask01 = 0x01010101UL;
inline constexpr unsigned long kStrlenMask80 = 0x80808080UL;
#endif

inline constexpr std::size_t kLongPtrMask = sizeof(unsigned long) - 1;

[[nodiscard]] inline std::size_t strlen(const char* str) noexcept {
    const char* p;
    const unsigned long* lp;
    long va;
    long vb;

    lp = reinterpret_cast<const unsigned long*>(
        reinterpret_cast<std::uintptr_t>(str) & ~kLongPtrMask);
    va = (*lp - kStrlenMask01);
    vb = ((~*lp) & kStrlenMask80);
    ++lp;
    if (va & vb) {
        for (p = str; p < reinterpret_cast<const char*>(lp); ++p) {
            if (*p == '\0') {
                return static_cast<std::size_t>(p - str);
            }
        }
    }

    for (;;) {
        va = (*lp - kStrlenMask01);
        vb = ((~*lp) & kStrlenMask80);
        if (va & vb) {
            p = reinterpret_cast<const char*>(lp);
            if (p[0] == '\0') {
                return static_cast<std::size_t>(p - str);
            }
            if (p[1] == '\0') {
                return static_cast<std::size_t>(p - str + 1);
            }
            if (p[2] == '\0') {
                return static_cast<std::size_t>(p - str + 2);
            }
            if (p[3] == '\0') {
                return static_cast<std::size_t>(p - str + 3);
            }
#if ULONG_MAX == 0xffffffffffffffffULL
            if (p[4] == '\0') {
                return static_cast<std::size_t>(p - str + 4);
            }
            if (p[5] == '\0') {
                return static_cast<std::size_t>(p - str + 5);
            }
            if (p[6] == '\0') {
                return static_cast<std::size_t>(p - str + 6);
            }
            if (p[7] == '\0') {
                return static_cast<std::size_t>(p - str + 7);
            }
#endif
        }
        ++lp;
    }
}

[[nodiscard]] inline int strcmp(const char* s1, const char* s2) noexcept {
    while (*s1 == *s2++) {
        if (*s1++ == '\0') {
            return 0;
        }
    }
    return static_cast<int>(static_cast<unsigned char>(*s1) -
                          static_cast<unsigned char>(*(s2 - 1)));
}

[[nodiscard]] inline int strncmp(const char* s1, const char* s2,
                                 std::size_t n) noexcept {
    if (n == 0) {
        return 0;
    }
    do {
        if (*s1 != *s2++) {
            return static_cast<int>(static_cast<unsigned char>(*s1) -
                                    static_cast<unsigned char>(*(s2 - 1)));
        }
        if (*s1++ == '\0') {
            break;
        }
    } while (--n != 0);
    return 0;
}

} // namespace pbsd::userland::libc
