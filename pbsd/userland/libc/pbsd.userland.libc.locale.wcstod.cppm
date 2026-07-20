module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstod;

export import pbsd.core;

/// wcstod scaffold from hbsd/src/lib/libc/locale/wcstod.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline double wcstod(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0.0;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0.0;
}

} // namespace pbsd::userland::libc::locale
