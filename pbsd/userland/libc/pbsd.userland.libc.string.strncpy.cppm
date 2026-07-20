module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strncpy;

export import pbsd.core;

/// strncpy from hbsd/src/lib/libc/string/strncpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* strncpy_copy(char* dst, const char* src, std::size_t n) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    std::size_t i = 0;
    for (; i < n && src[i] != '\0'; ++i) dst[i] = src[i];
    for (; i < n; ++i) dst[i] = '\0';
    return dst;
}

} // namespace pbsd::userland::libc
