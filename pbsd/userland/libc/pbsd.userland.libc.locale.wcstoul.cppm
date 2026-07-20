module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstoul;

export import pbsd.core;

/// wcstoul scaffold from hbsd/src/lib/libc/locale/wcstoul.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline unsigned long wcstoul(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0UL;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0UL;
}

} // namespace pbsd::userland::libc::locale
