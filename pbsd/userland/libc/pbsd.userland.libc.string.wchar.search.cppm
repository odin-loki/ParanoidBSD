module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.search;

/// wcslen/wcsrchr from hbsd/src/lib/libc/string/{wcslen,wcsrchr}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t wcslen(const wchar_t* s) noexcept {
    const wchar_t* p = s;
    while (*p != L'\0') {
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

[[nodiscard]] inline const wchar_t* wcsrchr(const wchar_t* s, wchar_t c) noexcept {
    const wchar_t* last = nullptr;
    while (*s != L'\0') {
        if (*s == c) {
            last = s;
        }
        ++s;
    }
    if (c == L'\0') {
        return s;
    }
    return last;
}

} // namespace pbsd::userland::libc
