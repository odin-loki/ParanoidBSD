module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstol;

export import pbsd.core;

/// wcstol scaffold from hbsd/src/lib/libc/locale/wcstol.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline long wcstol(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0L;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0L;
}

} // namespace pbsd::userland::libc::locale
