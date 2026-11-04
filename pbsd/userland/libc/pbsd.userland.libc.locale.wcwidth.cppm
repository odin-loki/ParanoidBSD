module;

#include <cwchar>

export module pbsd.userland.libc.locale.wcwidth;

/// wcwidth from hbsd/src/lib/libc/locale/wcwidth.c (ASCII subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wcwidth(wchar_t wc) noexcept {
    if (wc == L'\0') {
        return 0;
    }
    if (wc >= 0x20 && wc < 0x7f) {
        return 1;
    }
    if (wc == L'\t') {
        return -1;
    }
    return -1;
}

} // namespace pbsd::userland::libc
