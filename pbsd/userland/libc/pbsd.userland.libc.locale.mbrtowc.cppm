module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbrtowc;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbrtowc(wchar_t* pwc, const char* s, std::size_t n, mbstate_t* ps) noexcept {
    (void)ps;
    if (s == nullptr) {
        return 0;
    }
    if (n == 0) {
        return static_cast<std::size_t>(-2);
    }
    if (s[0] == '\0') {
        if (pwc != nullptr) {
            *pwc = L'\0';
        }
        return 0;
    }
    const unsigned char c = static_cast<unsigned char>(s[0]);
    if (c < 0x80) {
        if (pwc != nullptr) {
            *pwc = static_cast<wchar_t>(c);
        }
        return 1;
    }
    return static_cast<std::size_t>(-1);
}

} // namespace pbsd::userland::libc::locale
