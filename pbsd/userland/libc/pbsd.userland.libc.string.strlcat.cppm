module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strlcat;

export import pbsd.core;

/// strlcat from hbsd/src/lib/libc/string/strlcat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strlcat_append(char* dst, const char* src, std::size_t size) noexcept {
    if (dst == nullptr || src == nullptr) {
        return 0;
    }
    std::size_t dlen = 0;
    while (dlen < size && dst[dlen] != '\0') {
        ++dlen;
    }
    std::size_t i = 0;
    while (src[i] != '\0' && dlen + i + 1 < size) {
        dst[dlen + i] = src[i];
        ++i;
    }
    if (dlen < size) {
        dst[dlen + i] = '\0';
    }
    while (src[i] != '\0') {
        ++i;
    }
    return dlen + i;
}

} // namespace pbsd::userland::libc
