module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strpbrk;

export import pbsd.core;

/// strpbrk from hbsd/src/lib/libc/string/strpbrk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strpbrk_find(const char* s, const char* accept) noexcept {
    if (s == nullptr || accept == nullptr) return nullptr;
    for (; *s != '\0'; ++s) {
        for (const char* a = accept; *a != '\0'; ++a) {
            if (*s == *a) return const_cast<char*>(s);
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
