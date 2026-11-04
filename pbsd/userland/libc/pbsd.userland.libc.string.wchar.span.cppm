module;

#include <cwchar>

export module pbsd.userland.libc.string.wchar.span;

/// wcsspn/wcscspn from hbsd/src/lib/libc/string/{wcsspn,wcscspn}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t wcsspn(const wchar_t* s, const wchar_t* accept) noexcept {
    const wchar_t* p = s;
    while (*p != L'\0') {
        const wchar_t* a = accept;
        while (*a != L'\0' && *a != *p) {
            ++a;
        }
        if (*a == L'\0') {
            break;
        }
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

[[nodiscard]] inline std::size_t wcscspn(const wchar_t* s, const wchar_t* reject) noexcept {
    const wchar_t* p = s;
    while (*p != L'\0') {
        for (const wchar_t* r = reject; *r != L'\0'; ++r) {
            if (*r == *p) {
                return static_cast<std::size_t>(p - s);
            }
        }
        ++p;
    }
    return static_cast<std::size_t>(p - s);
}

} // namespace pbsd::userland::libc
