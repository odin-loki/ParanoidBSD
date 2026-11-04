module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.wcsrtombs;

import pbsd.userland.libc.locale.wcrtomb;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t wcsrtombs(char* dst, const wchar_t** src, std::size_t len, mbstate_t* ps) noexcept {
    if (src == nullptr || *src == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    while (count < len) {
        const std::size_t r = pbsd::userland::libc::locale::wcrtomb(dst != nullptr ? dst + count : nullptr, **src, ps);
        if (r == static_cast<std::size_t>(-1)) {
            return static_cast<std::size_t>(-1);
        }
        if (**src == L'\0') {
            return count;
        }
        ++(*src);
        count += r;
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
