module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.set;

/// wmemset/wmemchr from hbsd/src/lib/libc/string/{wmemset,wmemchr}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wmemset(wchar_t* dst, wchar_t c, std::size_t n) noexcept {
    wchar_t* p = dst;
    while (n-- > 0) {
        *p++ = c;
    }
    return dst;
}

[[nodiscard]] inline const wchar_t* wmemchr(const wchar_t* s, wchar_t c, std::size_t n) noexcept {
    while (n > 0) {
        if (*s == c) {
            return s;
        }
        ++s;
        --n;
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
