module;

#include <cerrno>
#include <climits>

export module pbsd.userland.libc.stdlib.strtoq;

export import pbsd.userland.libc.stdlib.convert.ext;

/// strtoq/strtouq from hbsd/src/lib/libc/stdlib/{strtoq,strtouq}.c (C-locale subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtoq(const char* nptr, char** endptr, int base) noexcept {
    return strtoll(nptr, endptr, base);
}

[[nodiscard]] inline unsigned long long strtouq(const char* nptr, char** endptr,
                                                  int base) noexcept {
    return strtoull(nptr, endptr, base);
}

} // namespace pbsd::userland::libc
