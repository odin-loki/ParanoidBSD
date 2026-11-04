module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstof;

export import pbsd.core;

/// wcstof scaffold from hbsd/src/lib/libc/locale/wcstof.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline float wcstof(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0.0f;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0.0f;
}

} // namespace pbsd::userland::libc::locale
