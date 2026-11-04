module;

#include <climits>
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.search;

import pbsd.userland.libc.string;

/// strchr/strrchr/strchrnul from hbsd/src/lib/libc/string/{strchr,strrchr,strchrnul}.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kSearchAlign = sizeof(std::size_t);
inline constexpr std::size_t kSearchOnes = static_cast<std::size_t>(-1) / UCHAR_MAX;
inline constexpr std::size_t kSearchHighs = kSearchOnes * (UCHAR_MAX / 2 + 1);

[[nodiscard]] inline bool search_has_zero(std::size_t x) noexcept {
    return ((x - kSearchOnes) & (~x) & kSearchHighs) != 0;
}

[[nodiscard]] inline char* strchrnul(const char* s, int c) noexcept {
    auto ch = static_cast<unsigned char>(c);
    if (ch == 0) {
        return const_cast<char*>(s + strlen(s));
    }

    while ((reinterpret_cast<std::uintptr_t>(s) % kSearchAlign) != 0) {
        if (*s == '\0' || static_cast<unsigned char>(*s) == ch) {
            return const_cast<char*>(s);
        }
        ++s;
    }

    const std::size_t k = kSearchOnes * ch;
    const auto* w = reinterpret_cast<const std::size_t*>(static_cast<const void*>(s));
    while (!search_has_zero(*w) && !search_has_zero(*w ^ k)) {
        ++w;
    }
    s = reinterpret_cast<const char*>(static_cast<const void*>(w));

    while (*s != '\0' && static_cast<unsigned char>(*s) != ch) {
        ++s;
    }
    return const_cast<char*>(s);
}

[[nodiscard]] inline char* strchr(const char* s, int c) noexcept {
    char* r = strchrnul(s, c);
    return static_cast<unsigned char>(*r) == static_cast<unsigned char>(c) ? r : nullptr;
}

[[nodiscard]] inline char* strrchr(const char* p, int ch) noexcept {
    char* save = nullptr;
    const auto c = static_cast<char>(ch);

    for (;; ++p) {
        if (*p == c) {
            save = const_cast<char*>(p);
        }
        if (*p == '\0') {
            return save;
        }
    }
}

} // namespace pbsd::userland::libc
