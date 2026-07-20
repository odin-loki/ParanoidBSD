module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcasecmp;

export import pbsd.core;

/// strcasecmp from hbsd/src/lib/libc/string/strcasecmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strcasecmp_bytes(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) return 0;
    while (*a && *b) {
        char ca = *a >= 'A' && *a <= 'Z' ? static_cast<char>(*a + 32) : *a;
        char cb = *b >= 'A' && *b <= 'Z' ? static_cast<char>(*b + 32) : *b;
        if (ca != cb) return static_cast<unsigned char>(ca) - static_cast<unsigned char>(cb);
        ++a; ++b;
    }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

} // namespace pbsd::userland::libc
