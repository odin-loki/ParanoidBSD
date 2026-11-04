module;
#include <cstddef>

export module pbsd.userland.libc.locale.utf8;

export import pbsd.core;

/// utf8 codec scaffold from hbsd/src/lib/libc/locale/utf8.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly utf8_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
