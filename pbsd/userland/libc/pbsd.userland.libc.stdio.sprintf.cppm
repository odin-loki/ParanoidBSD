module;

#include <cstddef>
#include <cstdio>

export module pbsd.userland.libc.stdio.sprintf;

/// sprintf/snprintf size helpers from hbsd/src/lib/libc/stdio/{sprintf,snprintf}.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int bounded_copy(char* dst, std::size_t size, const char* src) noexcept {
    if (dst == nullptr || size == 0) {
        return 0;
    }
    std::size_t i = 0;
    for (; i + 1 < size && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return static_cast<int>(i);
}

[[nodiscard]] inline int snprintf(char* str, std::size_t size, const char* fmt, ...) noexcept {
    (void)fmt;
    return bounded_copy(str, size, "");
}

} // namespace pbsd::userland::libc::stdio
