module;
#include <cwchar>
#include <cstddef>

export module pbsd.userland.libc.locale.wcstoull;

export import pbsd.core;

/// wcstoull scaffold from hbsd/src/lib/libc/locale/wcstoull.c
export namespace pbsd::userland::libc::locale {

[[nodiscard]] inline unsigned long long wcstoull(const wchar_t* nptr, wchar_t** endptr, int base) noexcept {
    (void)base;
    if (nptr == nullptr) {
        return 0ULL;
    }
    if (endptr != nullptr) {
        *endptr = const_cast<wchar_t*>(nptr);
    }
    return 0ULL;
}

} // namespace pbsd::userland::libc::locale
