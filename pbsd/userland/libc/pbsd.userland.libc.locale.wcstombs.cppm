module;

#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.wcstombs;

/// wcstombs stub from hbsd/src/lib/libc/locale/wcstombs.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t wcstombs(char* s, const wchar_t* pwcs, std::size_t n) noexcept {
    if (pwcs == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    for (; *pwcs != L'\0'; ++pwcs) {
        if (*pwcs > 0xff) {
            return static_cast<std::size_t>(-1);
        }
        if (s != nullptr) {
            if (count + 1 >= n) {
                return static_cast<std::size_t>(-1);
            }
            s[count] = static_cast<char>(*pwcs);
        }
        ++count;
    }
    if (s != nullptr && count < n) {
        s[count] = '\0';
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
