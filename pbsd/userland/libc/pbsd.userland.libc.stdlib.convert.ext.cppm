module;

#include <climits>
#include <cctype>
#include <cerrno>
#include <cstdlib>

export module pbsd.userland.libc.stdlib.convert.ext;

import pbsd.userland.libc.stdlib;

/// strtoll/strtoull from hbsd/src/lib/libc/stdlib/{strtoll,strtoull}.c (C locale)
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtoll(const char* nptr, char** endptr, int base) noexcept {
    const char* s;
    unsigned long long acc;
    char c;
    unsigned long long cutoff;
    int neg;
    int any;
    unsigned long long cutlim;

    s = nptr;
    do {
        c = *s++;
    } while (std::isspace(static_cast<unsigned char>(c)) != 0);

    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+') {
            c = *s++;
        }
    }

    if ((base == 0 || base == 16) && c == '0' &&
        (*s == 'x' || *s == 'X') &&
        ((s[1] >= '0' && s[1] <= '9') || (s[1] >= 'A' && s[1] <= 'F') ||
         (s[1] >= 'a' && s[1] <= 'f'))) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    acc = any = 0;
    if (base < 2 || base > 36) {
        goto noconv;
    }

    cutoff = neg ? static_cast<unsigned long long>(-(LLONG_MIN + LLONG_MAX)) + LLONG_MAX
                 : LLONG_MAX;
    cutlim = cutoff % static_cast<unsigned long long>(base);
    cutoff /= static_cast<unsigned long long>(base);

    for (;; c = *s++) {
        if (c >= '0' && c <= '9') {
            c = static_cast<char>(c - '0');
        } else if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 10);
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff ||
            (acc == cutoff && static_cast<unsigned long long>(c) > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= static_cast<unsigned long long>(base);
            acc += static_cast<unsigned long long>(c);
        }
    }

    if (any < 0) {
        acc = neg ? static_cast<unsigned long long>(LLONG_MIN) : LLONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv:
        errno = EINVAL;
    } else if (neg) {
        acc = static_cast<unsigned long long>(0ULL - acc);
    }

    if (endptr != nullptr) {
        *endptr = const_cast<char*>(any ? s - 1 : nptr);
    }
    return static_cast<long long>(acc);
}

[[nodiscard]] inline unsigned long long strtoull(const char* nptr, char** endptr,
                                                 int base) noexcept {
    const char* s;
    unsigned long long acc;
    char c;
    unsigned long long cutoff;
    int neg;
    int any;
    unsigned long long cutlim;

    s = nptr;
    do {
        c = *s++;
    } while (std::isspace(static_cast<unsigned char>(c)) != 0);

    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+') {
            c = *s++;
        }
    }

    if ((base == 0 || base == 16) && c == '0' &&
        (*s == 'x' || *s == 'X') &&
        ((s[1] >= '0' && s[1] <= '9') || (s[1] >= 'A' && s[1] <= 'F') ||
         (s[1] >= 'a' && s[1] <= 'f'))) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    acc = any = 0;
    if (base < 2 || base > 36) {
        goto noconv_ul;
    }

    cutoff = ULLONG_MAX / static_cast<unsigned long long>(base);
    cutlim = ULLONG_MAX % static_cast<unsigned long long>(base);

    for (;; c = *s++) {
        if (c >= '0' && c <= '9') {
            c = static_cast<char>(c - '0');
        } else if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 10);
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff ||
            (acc == cutoff && static_cast<unsigned long long>(c) > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= static_cast<unsigned long long>(base);
            acc += static_cast<unsigned long long>(c);
        }
    }

    if (any < 0) {
        acc = ULLONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv_ul:
        errno = EINVAL;
    } else if (neg) {
        acc = static_cast<unsigned long long>(-static_cast<long long>(acc));
    }

    if (endptr != nullptr) {
        *endptr = const_cast<char*>(any ? s - 1 : nptr);
    }
    return acc;
}

} // namespace pbsd::userland::libc
