module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcpy;

export import pbsd.core;

/// strcpy from hbsd/src/lib/libc/string/strcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strcpy_copy(char* dst, const char* src) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    char* d = dst;
    while ((*d++ = *src++) != '\0') {}
    return dst;
}

} // namespace pbsd::userland::libc
