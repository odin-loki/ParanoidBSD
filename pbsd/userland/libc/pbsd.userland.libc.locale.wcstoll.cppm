module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstoll;

export import pbsd.core;

/// wcstoll scaffold from hbsd/src/lib/libc/locale/wcstoll.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline long long wcstoll(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0LL;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0LL;
}

} // namespace pbsd::userland::libc::locale
