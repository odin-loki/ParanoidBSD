module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstoimax;

export import pbsd.core;

/// wcstoimax scaffold from hbsd/src/lib/libc/locale/wcstoimax.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline long long wcstoimax(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0;
}

} // namespace pbsd::userland::libc::locale
