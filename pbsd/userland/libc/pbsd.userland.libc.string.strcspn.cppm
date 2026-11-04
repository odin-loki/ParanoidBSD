module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcspn;

export import pbsd.core;

/// strcspn from hbsd/src/lib/libc/string/strcspn.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strcspn_span(const char* s, const char* reject) noexcept {
    if (s == nullptr || reject == nullptr) return 0;
    std::size_t n = 0;
    for (; s[n] != '\0'; ++n) {
        for (const char* r = reject; *r != '\0'; ++r) {
            if (s[n] == *r) return n;
        }
    }
    return n;
}

} // namespace pbsd::userland::libc
