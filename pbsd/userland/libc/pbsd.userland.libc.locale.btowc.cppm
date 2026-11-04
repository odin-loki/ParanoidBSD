module;

#include <cwchar>

export module pbsd.userland.libc.locale.btowc;

/// btowc from hbsd/src/lib/libc/locale/btowc.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline wint_t btowc(int c) noexcept {
    if (c == -1) { // EOF
        return WEOF;
    }
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc >= 0x80) {
        return WEOF;
    }
    return static_cast<wint_t>(uc);
}

} // namespace pbsd::userland::libc::locale
