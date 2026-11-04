module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbrtoc32;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbrtoc32(char32_t* pc32, const char* s, std::size_t n, mbstate_t* ps) noexcept {
    (void)ps;
    if (s == nullptr) {
        return 0;
    }
    if (n == 0) {
        return static_cast<std::size_t>(-2);
    }
    if (s[0] == '\0') {
        if (pc32 != nullptr) {
            *pc32 = U'\0';
        }
        return 0;
    }
    const unsigned char c = static_cast<unsigned char>(s[0]);
    if (c < 0x80) {
        if (pc32 != nullptr) {
            *pc32 = static_cast<char32_t>(c);
        }
        return 1;
    }
    return static_cast<std::size_t>(-1);
}

} // namespace pbsd::userland::libc::locale
