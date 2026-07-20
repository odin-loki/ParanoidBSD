module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.wctomb;

import pbsd.userland.libc.locale.wcrtomb;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline int wctomb(char* s, wchar_t wc) noexcept {
    if (s == nullptr) {
        return 0;
    }
    const std::size_t r = pbsd::userland::libc::locale::wcrtomb(s, wc, nullptr);
    if (r == static_cast<std::size_t>(-1)) {
        return -1;
    }
    return static_cast<int>(r);
}

} // namespace pbsd::userland::libc::locale
