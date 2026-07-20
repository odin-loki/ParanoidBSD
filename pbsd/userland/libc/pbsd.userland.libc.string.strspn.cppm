module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strspn;

export import pbsd.core;

/// strspn from hbsd/src/lib/libc/string/strspn.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strspn_span(const char* s, const char* accept) noexcept {
    if (s == nullptr || accept == nullptr) return 0;
    std::size_t n = 0;
    for (; s[n] != '\0'; ++n) {
        bool ok = false;
        for (const char* a = accept; *a != '\0'; ++a) {
            if (s[n] == *a) { ok = true; break; }
        }
        if (!ok) break;
    }
    return n;
}

} // namespace pbsd::userland::libc
