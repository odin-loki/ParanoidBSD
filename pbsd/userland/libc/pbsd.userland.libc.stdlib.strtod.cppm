module;

#include <cctype>
#include <cerrno>
#include <cstdlib>

export module pbsd.userland.libc.stdlib.strtod;

import pbsd.userland.libc.stdlib;

/// strtod from hbsd/src/lib/libc/stdlib/strtod.c (C-locale decimal subset)
export namespace pbsd::userland::libc {

[[nodiscard]] inline double strtod(const char* nptr, char** endptr) noexcept {
    const char* s = nptr;
    char c;
    int neg = 0;
    double acc = 0.0;
    int any = 0;

    do {
        c = *s++;
    } while (std::isspace(static_cast<unsigned char>(c)) != 0);

    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }

    while (c >= '0' && c <= '9') {
        any = 1;
        acc = acc * 10.0 + static_cast<double>(c - '0');
        c = *s++;
    }

    if (c == '.') {
        double factor = 0.1;
        while ((c = *s++) >= '0' && c <= '9') {
            any = 1;
            acc += static_cast<double>(c - '0') * factor;
            factor *= 0.1;
        }
    }

    if (c == 'e' || c == 'E') {
        int eneg = 0;
        long exp = 0;
        c = *s++;
        if (c == '-') {
            eneg = 1;
            c = *s++;
        } else if (c == '+') {
            c = *s++;
        }
        while (c >= '0' && c <= '9') {
            any = 1;
            exp = exp * 10 + (c - '0');
            c = *s++;
        }
        double scale = 1.0;
        for (long i = 0; i < exp; ++i) {
            scale *= 10.0;
        }
        acc = eneg ? acc / scale : acc * scale;
    }

    if (!any) {
        errno = EINVAL;
        if (endptr != nullptr) {
            *endptr = const_cast<char*>(nptr);
        }
        return 0.0;
    }

    if (endptr != nullptr) {
        *endptr = const_cast<char*>(s - 1);
    }
    return neg ? -acc : acc;
}

} // namespace pbsd::userland::libc
