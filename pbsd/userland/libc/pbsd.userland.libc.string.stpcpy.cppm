module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.stpcpy;

export import pbsd.core;

/// stpcpy from hbsd/src/lib/libc/string/stpcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* stpcpy_copy(char* dst, const char* src) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    while ((*dst++ = *src++) != '\0') {}
    return dst - 1;
}

} // namespace pbsd::userland::libc
