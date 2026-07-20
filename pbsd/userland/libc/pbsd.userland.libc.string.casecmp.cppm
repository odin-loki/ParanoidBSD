module;

#include <cctype>
#include <cstddef>

export module pbsd.userland.libc.string.casecmp;

/// strcasecmp/strncasecmp from hbsd/src/lib/libc/string/strcasecmp.c (C locale)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strcasecmp(const char* s1, const char* s2) noexcept {
    const auto* us1 = reinterpret_cast<const unsigned char*>(s1);
    const auto* us2 = reinterpret_cast<const unsigned char*>(s2);
    while (std::tolower(*us1) == std::tolower(*us2++)) {
        if (*us1++ == '\0') {
            return 0;
        }
    }
    return std::tolower(*us1) - std::tolower(*(--us2));
}

[[nodiscard]] inline int strncasecmp(const char* s1, const char* s2, std::size_t n) noexcept {
    if (n != 0) {
        const auto* us1 = reinterpret_cast<const unsigned char*>(s1);
        const auto* us2 = reinterpret_cast<const unsigned char*>(s2);
        do {
            if (std::tolower(*us1) != std::tolower(*us2++)) {
                return std::tolower(*us1) - std::tolower(*(--us2));
            }
            if (*us1++ == '\0') {
                break;
            }
        } while (--n != 0);
    }
    return 0;
}

} // namespace pbsd::userland::libc
