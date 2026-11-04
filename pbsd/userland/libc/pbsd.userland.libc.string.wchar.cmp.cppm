module;

#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.string.wchar.cmp;

/// wcscmp/wcsncmp from hbsd/src/lib/libc/string/{wcscmp,wcsncmp}.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wcscmp(const wchar_t* a, const wchar_t* b) noexcept {
    while (*a != L'\0' && *a == *b) {
        ++a;
        ++b;
    }
    return static_cast<int>(*a) - static_cast<int>(*b);
}

[[nodiscard]] inline int wcsncmp(const wchar_t* a, const wchar_t* b, std::size_t n) noexcept {
    while (n > 0 && *a != L'\0' && *a == *b) {
        ++a;
        ++b;
        --n;
    }
    if (n == 0) {
        return 0;
    }
    return static_cast<int>(*a) - static_cast<int>(*b);
}

} // namespace pbsd::userland::libc
