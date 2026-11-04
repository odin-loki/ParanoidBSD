module;

#include <climits>
#include <cstddef>

export module pbsd.userland.libc.string.span;

/// strspn/strcspn from hbsd/src/lib/libc/string/{strspn,strcspn}.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kSpanTblWords =
    (UCHAR_MAX + 1) / (CHAR_BIT * static_cast<int>(sizeof(unsigned long)));

[[nodiscard]] inline std::size_t strspn(const char* s, const char* charset) noexcept {
    const char* s1;
    unsigned long bit;
    unsigned long tbl[kSpanTblWords];
    int idx;

    if (*s == '\0') {
        return 0;
    }

#if LONG_BIT == 64
    tbl[3] = tbl[2] = tbl[1] = tbl[0] = 0;
#else
    for (idx = 0; idx < static_cast<int>(kSpanTblWords); ++idx) {
        tbl[idx] = 0;
    }
#endif
    for (; *charset != '\0'; ++charset) {
        idx = static_cast<unsigned char>(*charset) /
              (CHAR_BIT * static_cast<int>(sizeof(unsigned long)));
        bit = 1UL << (static_cast<unsigned char>(*charset) %
                      (CHAR_BIT * static_cast<int>(sizeof(unsigned long))));
        tbl[idx] |= bit;
    }

    for (s1 = s;; ++s1) {
        idx = static_cast<unsigned char>(*s1) /
              (CHAR_BIT * static_cast<int>(sizeof(unsigned long)));
        bit = 1UL << (static_cast<unsigned char>(*s1) %
                      (CHAR_BIT * static_cast<int>(sizeof(unsigned long))));
        if ((tbl[idx] & bit) == 0) {
            break;
        }
    }
    return static_cast<std::size_t>(s1 - s);
}

[[nodiscard]] inline std::size_t strcspn(const char* s, const char* charset) noexcept {
    const char* s1;
    unsigned long bit;
    unsigned long tbl[kSpanTblWords];
    int idx;

    if (*s == '\0') {
        return 0;
    }

#if LONG_BIT == 64
    tbl[0] = 1;
    tbl[3] = tbl[2] = tbl[1] = 0;
#else
    tbl[0] = 1;
    for (idx = 1; idx < static_cast<int>(kSpanTblWords); ++idx) {
        tbl[idx] = 0;
    }
#endif
    for (; *charset != '\0'; ++charset) {
        idx = static_cast<unsigned char>(*charset) /
              (CHAR_BIT * static_cast<int>(sizeof(unsigned long)));
        bit = 1UL << (static_cast<unsigned char>(*charset) %
                      (CHAR_BIT * static_cast<int>(sizeof(unsigned long))));
        tbl[idx] |= bit;
    }

    for (s1 = s;; ++s1) {
        idx = static_cast<unsigned char>(*s1) /
              (CHAR_BIT * static_cast<int>(sizeof(unsigned long)));
        bit = 1UL << (static_cast<unsigned char>(*s1) %
                      (CHAR_BIT * static_cast<int>(sizeof(unsigned long))));
        if ((tbl[idx] & bit) != 0) {
            break;
        }
    }
    return static_cast<std::size_t>(s1 - s);
}

} // namespace pbsd::userland::libc
