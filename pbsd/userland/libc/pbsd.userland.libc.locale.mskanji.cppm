module;
#include <cstddef>

export module pbsd.userland.libc.locale.mskanji;

export import pbsd.core;

/// mskanji codec scaffold from hbsd/src/lib/libc/locale/mskanji.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly mskanji_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
