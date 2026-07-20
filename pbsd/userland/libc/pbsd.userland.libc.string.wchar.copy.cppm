module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.copy;

/// wcpcpy/wcpncpy from hbsd/src/lib/libc/string/{wcpcpy,wcpncpy}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline wchar_t* wcpcpy(wchar_t* dst, const wchar_t* src) noexcept {
    while ((*dst = *src) != L'\0') {
        ++dst;
        ++src;
    }
    return dst;
}

[[nodiscard]] inline wchar_t* wcpncpy(wchar_t* dst, const wchar_t* src, std::size_t n) noexcept {
    wchar_t* d = dst;
    while (n > 0 && *src != L'\0') {
        *d++ = *src++;
        --n;
    }
    while (n > 0) {
        *d++ = L'\0';
        --n;
    }
    return dst;
}

} // namespace pbsd::userland::libc
