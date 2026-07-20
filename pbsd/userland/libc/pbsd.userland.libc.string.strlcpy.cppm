module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strlcpy;

export import pbsd.core;

/// strlcpy from hbsd/src/lib/libc/string/strlcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strlcpy_copy(char* dst, const char* src, std::size_t size) noexcept {
    if (dst == nullptr || src == nullptr || size == 0) {
        return src == nullptr ? 0 : 0;
    }
    std::size_t i = 0;
    for (; i + 1 < size && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    if (size > 0) {
        dst[i] = '\0';
    }
    while (src[i] != '\0') {
        ++i;
    }
    return i;
}

} // namespace pbsd::userland::libc
