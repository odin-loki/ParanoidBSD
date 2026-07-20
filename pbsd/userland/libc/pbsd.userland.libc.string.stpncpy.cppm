module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.stpncpy;

export import pbsd.core;

/// stpncpy from hbsd/src/lib/libc/string/stpncpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* stpncpy_copy(char* dst, const char* src, std::size_t n) noexcept {
    if (dst == nullptr || src == nullptr) return dst;
    std::size_t i = 0;
    for (; i < n && src[i] != '\0'; ++i) dst[i] = src[i];
    for (std::size_t j = i; j < n; ++j) dst[j] = '\0';
    return dst + i;
}

} // namespace pbsd::userland::libc
