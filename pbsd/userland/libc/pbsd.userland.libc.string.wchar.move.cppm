module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.move;

/// wmemmove/wmemcpy from hbsd/src/lib/libc/string/{wmemmove,wmemcpy}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemcpy(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    wchar_t* d = dst;
    while (n-- > 0) {
        *d++ = *src++;
    }
    return dst;
}

[[nodiscard]] inline wchar_t* wmemmove(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    if (dst <= src || dst >= src + n) {
        return wmemcpy(dst, src, n);
    }
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
    return dst;
}

} // namespace pbsd::userland::libc
