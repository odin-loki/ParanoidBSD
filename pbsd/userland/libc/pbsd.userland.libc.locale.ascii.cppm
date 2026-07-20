module;
#include <cstddef>

export module pbsd.userland.libc.locale.ascii;

export import pbsd.core;

/// ascii codec scaffold from hbsd/src/lib/libc/locale/ascii.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline StatusOnly ascii_mbtowc(wchar_t* wc, const char* s, std::size_t n) noexcept {
    (void)s;
    (void)n;
    if (wc != nullptr) {
        *wc = L'\0';
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
