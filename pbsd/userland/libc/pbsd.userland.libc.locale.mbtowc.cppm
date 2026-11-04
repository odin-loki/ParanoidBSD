module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbtowc;

import pbsd.userland.libc.locale.mbrtowc;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int mbtowc(wchar_t* pwc, const char* s, std::size_t n) noexcept {
    if (s == nullptr) {
        return 0;
    }
    const std::size_t r = pbsd::userland::libc::locale::mbrtowc(pwc, s, n, nullptr);
    if (r == static_cast<std::size_t>(-1)) {
        return -1;
    }
    if (r == static_cast<std::size_t>(-2)) {
        return -2;
    }
    return static_cast<int>(r);
}

} // namespace pbsd::userland::libc::locale
