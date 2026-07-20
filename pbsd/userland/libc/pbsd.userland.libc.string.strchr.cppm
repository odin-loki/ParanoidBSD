module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strchr;

export import pbsd.core;

/// strchr from hbsd/src/lib/libc/string/strchr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strchr_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    for (; *s != '\0'; ++s) {
        if (*s == ch) return const_cast<char*>(s);
    }
    return ch == '\0' ? const_cast<char*>(s) : nullptr;
}

} // namespace pbsd::userland::libc
