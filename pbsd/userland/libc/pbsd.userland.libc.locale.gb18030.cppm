module;
#include <cstddef>

export module pbsd.userland.libc.locale.gb18030;

export import pbsd.core;

/// gb18030 codec scaffold from hbsd/src/lib/libc/locale/gb18030.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly gb18030_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
