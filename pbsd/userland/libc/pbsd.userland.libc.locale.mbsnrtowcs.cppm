module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.libc.locale.mbsnrtowcs;

import pbsd.userland.libc.locale.mbrtowc;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t mbsnrtowcs(wchar_t* dst, const char** src, std::size_t nms, std::size_t len, mbstate_t* ps) noexcept {
    if (src == nullptr || *src == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    std::size_t consumed = 0;
    while (count < len && consumed < nms) {
        wchar_t wc = L'\0';
        const std::size_t r = pbsd::userland::libc::locale::mbrtowc(&wc, *src, nms - consumed, ps);
        if (r == 0) {
            if (dst != nullptr) {
                dst[count] = L'\0';
            }
            ++count;
            break;
        }
        if (r == static_cast<std::size_t>(-1) || r == static_cast<std::size_t>(-2)) {
            return static_cast<std::size_t>(-1);
        }
        if (dst != nullptr) {
            dst[count] = wc;
        }
        *src += r;
        consumed += r;
        ++count;
    }
    return count;
}

} // namespace pbsd::userland::libc::locale
