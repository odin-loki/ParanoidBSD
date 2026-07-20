module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strrchr;

export import pbsd.core;

/// strrchr from hbsd/src/lib/libc/string/strrchr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strrchr_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    const char* last = nullptr;
    for (; *s != '\0'; ++s) {
        if (*s == ch) last = s;
    }
    if (ch == '\0') return const_cast<char*>(s);
    return const_cast<char*>(last);
}

} // namespace pbsd::userland::libc
