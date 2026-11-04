module;

#include <cwchar>

export module pbsd.userland.libc.locale.wctob;

/// wctob from hbsd/src/lib/libc/locale/wctob.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int wctob(wint_t wc) noexcept {
    if (wc == WEOF || wc > 0xff) {
        return -1; // EOF
    }
    return static_cast<int>(wc);
}

} // namespace pbsd::userland::libc::locale
