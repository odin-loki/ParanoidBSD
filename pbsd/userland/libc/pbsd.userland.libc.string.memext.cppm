module;

#include <cstddef>

export module pbsd.userland.libc.string.memext;

import pbsd.userland.libc.memory;

/// memrchr/memmem from hbsd/src/lib/libc/string/{memrchr,memmem}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memrchr(const void* s, int c, std::size_t n) noexcept {
    const auto* cp = static_cast<const unsigned char*>(s);
    const auto ch = static_cast<unsigned char>(c);

    if (n != 0) {
        cp += n;
        do {
            if (*(--cp) == ch) {
                return const_cast<unsigned char*>(cp);
            }
        } while (--n != 0);
    }
    return nullptr;
}

[[nodiscard]] inline void* memmem(const void* haystack, std::size_t haystacklen,
                                  const void* needle, std::size_t needlelen) noexcept {
    if (needlelen == 0) {
        return const_cast<void*>(haystack);
    }
    if (needlelen > haystacklen) {
        return nullptr;
    }
    const auto* h = static_cast<const unsigned char*>(haystack);
    const auto* n = static_cast<const unsigned char*>(needle);
    const std::size_t last = haystacklen - needlelen;
    for (std::size_t i = 0; i <= last; ++i) {
        if (h[i] == n[0] && memcmp(h + i, n, needlelen) == 0) {
            return const_cast<unsigned char*>(h + i);
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
