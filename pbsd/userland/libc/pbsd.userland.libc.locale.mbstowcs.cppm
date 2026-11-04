module;

#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbstowcs;

import pbsd.userland.libc.locale.mblen;

/// mbstowcs stub from hbsd/src/lib/libc/locale/mbstowcs.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbstowcs(wchar_t* pwcs, const char* s, std::size_t n) noexcept {
    if (s == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    while (*s != '\0') {
        const int len = pbsd::userland::libc::locale::mblen(s, n);
        if (len <= 0) {
            return static_cast<std::size_t>(-1);
        }
        if (pwcs != nullptr && count < n) {
            pwcs[count] = static_cast<wchar_t>(static_cast<unsigned char>(*s));
        }
        ++count;
        s += len;
    }
    if (pwcs != nullptr && count < n) {
        pwcs[count] = L'\0';
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
