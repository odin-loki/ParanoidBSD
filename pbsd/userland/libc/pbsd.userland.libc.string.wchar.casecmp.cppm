module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.casecmp;

/// wcscasecmp/wcsncasecmp from hbsd/src/lib/libc/string/{wcscasecmp,wcsncasecmp}.c
export namespace pbsd::userland::libc {

namespace detail {
[[nodiscard]] inline wchar_t towlower_ascii(wchar_t c) noexcept {
    return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c - L'A' + L'a') : c;
}
} // namespace detail

[[nodiscard]] inline int wcscasecmp(const wchar_t* a, const wchar_t* b) noexcept {
    while (*a != L'\0' && *b != L'\0') {
        const wchar_t ca = detail::towlower_ascii(*a);
        const wchar_t cb = detail::towlower_ascii(*b);
        if (ca != cb) {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        ++a;
        ++b;
    }
    return static_cast<int>(detail::towlower_ascii(*a)) -
           static_cast<int>(detail::towlower_ascii(*b));
}

[[nodiscard]] inline int wcsncasecmp(const wchar_t* a, const wchar_t* b, std::size_t n) noexcept {
    while (n > 0 && *a != L'\0' && *b != L'\0') {
        const wchar_t ca = detail::towlower_ascii(*a);
        const wchar_t cb = detail::towlower_ascii(*b);
        if (ca != cb) {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        ++a;
        ++b;
        --n;
    }
    if (n == 0) {
        return 0;
    }
    return static_cast<int>(detail::towlower_ascii(*a)) -
           static_cast<int>(detail::towlower_ascii(*b));
}

} // namespace pbsd::userland::libc
