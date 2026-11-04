module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.wcrtomb;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t wcrtomb(char* s, wchar_t wc, mbstate_t* ps) noexcept {
    (void)ps;
    if (s == nullptr) {
        return 0;
    }
    if (wc == L'\0') {
        s[0] = '\0';
        return 1;
    }
    if (static_cast<unsigned>(wc) < 0x80u) {
        s[0] = static_cast<char>(wc);
        return 1;
    }
    return static_cast<std::size_t>(-1);
}

} // namespace pbsd::userland::libc::locale
