module;
#include <cstddef>

export module pbsd.userland.libc.locale.euc;

export import pbsd.core;

/// euc codec scaffold from hbsd/src/lib/libc/locale/euc.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly euc_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
