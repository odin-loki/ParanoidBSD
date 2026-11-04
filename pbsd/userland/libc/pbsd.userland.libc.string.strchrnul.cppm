module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strchrnul;

export import pbsd.core;

/// strchrnul from hbsd/src/lib/libc/string/strchrnul.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strchrnul_find(const char* s, int c) noexcept {
    if (s == nullptr) return nullptr;
    const char ch = static_cast<char>(c);
    for (; *s != '\0'; ++s) {
        if (*s == ch) break;
    }
    return const_cast<char*>(s);
}

} // namespace pbsd::userland::libc
