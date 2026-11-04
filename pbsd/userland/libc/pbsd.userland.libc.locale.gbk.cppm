module;
#include <cstddef>

export module pbsd.userland.libc.locale.gbk;

export import pbsd.core;

/// gbk codec scaffold from hbsd/src/lib/libc/locale/gbk.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly gbk_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
