module;
#include <cstddef>
#include <ctime>
#include <cwchar>

export module pbsd.userland.libc.locale.wcsftime;

export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline std::size_t wcsftime(wchar_t* wcs, std::size_t max, const wchar_t* fmt, const std::tm* tm) noexcept {
    (void)fmt;
    (void)tm;
    if (wcs == nullptr || max == 0) {
        return 0;
    }
    wcs[0] = L'\0';
    return 0;
}

} // namespace pbsd::userland::libc::locale
