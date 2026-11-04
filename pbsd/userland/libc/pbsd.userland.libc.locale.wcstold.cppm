module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstold;

export import pbsd.core;

/// wcstold scaffold from hbsd/src/lib/libc/locale/wcstold.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline long double wcstold(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0.0L;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0.0L;
}

} // namespace pbsd::userland::libc::locale
