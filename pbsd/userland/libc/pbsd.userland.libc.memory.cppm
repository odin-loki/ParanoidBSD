module;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>

export module pbsd.userland.libc.memory;

/// Freestanding string/memory from hbsd/src/lib/libc/string/{bzero,memset,memcpy}.c
///
/// ABI notes:
/// - Algorithms match Berkeley libcsrc; word width follows `unsigned long`.
/// - Hosted test builds use inline wrappers in namespace pbsd::userland::libc.
///   Do not export `extern "C"` symbols here — kernel/userland link scripts
///   will provide the stable C ABI entry points when integrated.
/// - Overlap: memcpy() is undefined on overlap (use memmove when ported).
export namespace pbsd::userland::libc {

using word = std::intptr_t;
inline constexpr std::size_t kWordSize = sizeof(word);
inline constexpr std::size_t kWordMask = kWordSize - 1;

inline void bzero(void* dst0, std::size_t length) noexcept {
    auto* dst = static_cast<unsigned char*>(dst0);
    if (length < 3 * kWordSize) {
        while (length != 0) {
            *dst++ = 0;
            --length;
        }
        return;
    }

    if (auto t = reinterpret_cast<std::uintptr_t>(dst) & kWordMask; t != 0) {
        t = kWordSize - t;
        length -= t;
        do {
            *dst++ = 0;
        } while (--t != 0);
    }

    std::size_t t = length / kWordSize;
    do {
        *reinterpret_cast<unsigned long*>(static_cast<void*>(dst)) = 0;
        dst += kWordSize;
    } while (--t != 0);

    t = length & kWordMask;
    if (t != 0) {
        do {
            *dst++ = 0;
        } while (--t != 0);
    }
}

inline void* memset(void* dst0, int c0, std::size_t length) noexcept {
    auto* dst = static_cast<unsigned char*>(dst0);
    const auto c = static_cast<unsigned char>(c0);

    if (length < 3 * kWordSize) {
        while (length != 0) {
            *dst++ = c;
            --length;
        }
        return dst0;
    }

    unsigned long fill = c;
    if (fill != 0) {
        fill = (fill << 8) | fill;
#if ULONG_MAX > 0xffff
        fill = (fill << 16) | fill;
#endif
#if ULONG_MAX > 0xffffffff
        fill = (fill << 32) | fill;
#endif
    }

    if (auto t = reinterpret_cast<std::uintptr_t>(dst) & kWordMask; t != 0) {
        t = kWordSize - t;
        length -= t;
        do {
            *dst++ = c;
        } while (--t != 0);
    }

    std::size_t t = length / kWordSize;
    do {
        *reinterpret_cast<unsigned long*>(static_cast<void*>(dst)) = fill;
        dst += kWordSize;
    } while (--t != 0);

    t = length & kWordMask;
    if (t != 0) {
        do {
            *dst++ = c;
        } while (--t != 0);
    }
    return dst0;
}

inline void* memcpy(void* dst0, const void* src0, std::size_t length) noexcept {
    auto* dst = static_cast<char*>(dst0);
    const auto* src = static_cast<const char*>(src0);

    if (length == 0 || dst == src) {
        return dst0;
    }

#define TLOOP(s) \
    if (t)       \
        do {     \
            s;   \
        } while (--t)

    std::size_t t;
    if (reinterpret_cast<std::uintptr_t>(dst) <
        reinterpret_cast<std::uintptr_t>(src)) {
        t = reinterpret_cast<std::uintptr_t>(src);
        if ((t | reinterpret_cast<std::uintptr_t>(dst)) & kWordMask) {
            if ((t ^ reinterpret_cast<std::uintptr_t>(dst)) & kWordMask ||
                length < kWordSize) {
                t = length;
            } else {
                t = kWordSize - (t & kWordMask);
            }
            length -= t;
            TLOOP(*dst++ = *src++);
        }
        t = length / kWordSize;
        TLOOP(*reinterpret_cast<word*>(static_cast<void*>(dst)) =
                  *reinterpret_cast<const word*>(static_cast<const void*>(src));
              src += kWordSize;
              dst += kWordSize);
        t = length & kWordMask;
        TLOOP(*dst++ = *src++);
    } else {
        src += length;
        dst += length;
        t = reinterpret_cast<std::uintptr_t>(src);
        if ((t | reinterpret_cast<std::uintptr_t>(dst)) & kWordMask) {
            if ((t ^ reinterpret_cast<std::uintptr_t>(dst)) & kWordMask ||
                length <= kWordSize) {
                t = length;
            } else {
                t &= kWordMask;
            }
            length -= t;
            TLOOP(*--dst = *--src);
        }
        t = length / kWordSize;
        TLOOP(src -= kWordSize;
              dst -= kWordSize;
              *reinterpret_cast<word*>(static_cast<void*>(dst)) =
                  *reinterpret_cast<const word*>(static_cast<const void*>(src)));
        t = length & kWordMask;
        TLOOP(*--dst = *--src);
    }

#undef TLOOP
    return dst0;
}

/// memmove — overlap-safe copy from hbsd/src/lib/libc/string/bcopy.c (MEMMOVE).
inline void* memmove(void* dst0, const void* src0, std::size_t length) noexcept {
    auto* dst = static_cast<char*>(dst0);
    const auto* src = static_cast<const char*>(src0);

    if (length == 0 || dst == src) {
        return dst0;
    }

#define TLOOP(s) \
    if (t)       \
        do {     \
            s;   \
        } while (--t)

    std::size_t t;
    if (reinterpret_cast<std::uintptr_t>(dst) <
        reinterpret_cast<std::uintptr_t>(src)) {
        t = reinterpret_cast<std::uintptr_t>(src);
        if ((t | reinterpret_cast<std::uintptr_t>(dst)) & kWordMask) {
            if ((t ^ reinterpret_cast<std::uintptr_t>(dst)) & kWordMask ||
                length < kWordSize) {
                t = length;
            } else {
                t = kWordSize - (t & kWordMask);
            }
            length -= t;
            TLOOP(*dst++ = *src++);
        }
        t = length / kWordSize;
        TLOOP(*reinterpret_cast<word*>(static_cast<void*>(dst)) =
                  *reinterpret_cast<const word*>(static_cast<const void*>(src));
              src += kWordSize;
              dst += kWordSize);
        t = length & kWordMask;
        TLOOP(*dst++ = *src++);
    } else {
        src += length;
        dst += length;
        t = reinterpret_cast<std::uintptr_t>(src);
        if ((t | reinterpret_cast<std::uintptr_t>(dst)) & kWordMask) {
            if ((t ^ reinterpret_cast<std::uintptr_t>(dst)) & kWordMask ||
                length <= kWordSize) {
                t = length;
            } else {
                t &= kWordMask;
            }
            length -= t;
            TLOOP(*--dst = *--src);
        }
        t = length / kWordSize;
        TLOOP(src -= kWordSize;
              dst -= kWordSize;
              *reinterpret_cast<word*>(static_cast<void*>(dst)) =
                  *reinterpret_cast<const word*>(static_cast<const void*>(src)));
        t = length & kWordMask;
        TLOOP(*--dst = *--src);
    }

#undef TLOOP
    return dst0;
}

[[nodiscard]] inline int memcmp(const void* s1, const void* s2, std::size_t n) noexcept {
    if (n != 0) {
        const auto* p1 = static_cast<const unsigned char*>(s1);
        const auto* p2 = static_cast<const unsigned char*>(s2);
        do {
            if (*p1++ != *p2++) {
                return static_cast<int>(*--p1 - *--p2);
            }
        } while (--n != 0);
    }
    return 0;
}

inline constexpr std::size_t kMemchrWordSize = sizeof(std::size_t);
inline constexpr std::size_t kMemchrAlign = kMemchrWordSize - 1;
inline constexpr std::size_t kMemchrOnes = static_cast<std::size_t>(-1) / UCHAR_MAX;
inline constexpr std::size_t kMemchrHighs = kMemchrOnes * (UCHAR_MAX / 2 + 1);

[[nodiscard]] inline bool memchr_has_zero(std::size_t x) noexcept {
    return ((x - kMemchrOnes) & (~x) & kMemchrHighs) != 0;
}

[[nodiscard]] inline void* memchr(const void* src, int c, std::size_t n) noexcept {
    const auto* s = static_cast<const unsigned char*>(src);
    const auto ch = static_cast<unsigned char>(c);

    while (n != 0 && (reinterpret_cast<std::uintptr_t>(s) & kMemchrAlign) != 0) {
        if (*s == ch) {
            return const_cast<unsigned char*>(s);
        }
        ++s;
        --n;
    }

    if (n != 0) {
        const std::size_t k = kMemchrOnes * ch;
        const auto* w = reinterpret_cast<const std::size_t*>(static_cast<const void*>(s));
        while (n >= kMemchrWordSize && !memchr_has_zero(*w) && !memchr_has_zero(*w ^ k)) {
            w++;
            n -= kMemchrWordSize;
        }
        s = reinterpret_cast<const unsigned char*>(static_cast<const void*>(w));
    }

    while (n != 0 && *s != ch) {
        ++s;
        --n;
    }
    return n != 0 ? const_cast<unsigned char*>(s) : nullptr;
}

} // namespace pbsd::userland::libc
