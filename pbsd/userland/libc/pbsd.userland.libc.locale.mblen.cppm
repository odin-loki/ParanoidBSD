module;

#include <cstddef>

export module pbsd.userland.libc.locale.mblen;

/// mblen stub from hbsd/src/lib/libc/locale/mblen.c (UTF-8 C locale)
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int mblen(const char* s, std::size_t n) noexcept {
    if (s == nullptr) {
        return 0;
    }
    if (n == 0 || s[0] == '\0') {
        return 0;
    }
    const unsigned char c = static_cast<unsigned char>(s[0]);
    if (c < 0x80) {
        return 1;
    }
    return -1;
}

} // namespace pbsd::userland::libc::locale
