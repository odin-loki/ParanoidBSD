module;

#include <climits>
#include <cerrno>
#include <cstdlib>

export module pbsd.userland.libc.stdlib.strtonum;

import pbsd.userland.libc.stdlib.convert.ext;

/// strtonum from hbsd/src/lib/libc/stdlib/strtonum.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtonum(const char* numstr, long long minval, long long maxval,
                                         const char** errstrp) noexcept {
    if (minval > maxval) {
        if (errstrp != nullptr) {
            *errstrp = "invalid";
        }
        errno = EINVAL;
        return 0;
    }
    char* ep = nullptr;
    errno = 0;
    const long long ll = strtoll(numstr, &ep, 10);
    if (errno == EINVAL || numstr == ep || (ep != nullptr && *ep != '\0')) {
        if (errstrp != nullptr) {
            *errstrp = "invalid";
        }
        errno = EINVAL;
        return 0;
    }
    if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval) {
        if (errstrp != nullptr) {
            *errstrp = "too small";
        }
        errno = ERANGE;
        return 0;
    }
    if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval) {
        if (errstrp != nullptr) {
            *errstrp = "too large";
        }
        errno = ERANGE;
        return 0;
    }
    if (errstrp != nullptr) {
        *errstrp = nullptr;
    }
    return ll;
}

} // namespace pbsd::userland::libc
