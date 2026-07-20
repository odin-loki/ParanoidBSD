module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.c32rtomb;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t c32rtomb(char* s, char32_t c32, mbstate_t* ps) noexcept {
    (void)ps;
    if (s == nullptr) {
        return 0;
    }
    if (c32 == U'\0') {
        s[0] = '\0';
        return 1;
    }
    if (c32 < 0x80u) {
        s[0] = static_cast<char>(c32);
        return 1;
    }
    return static_cast<std::size_t>(-1);
}

} // namespace pbsd::userland::libc::locale
