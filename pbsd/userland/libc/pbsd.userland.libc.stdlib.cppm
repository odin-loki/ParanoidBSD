module;

#include <climits>
#include <cctype>
#include <cerrno>
#include <cstdlib>

export module pbsd.userland.libc.stdlib;

/// strtol/atoi from hbsd/src/lib/libc/stdlib/{strtol,atoi}.c (C-locale, no xlocale).
export namespace pbsd::userland::libc {

[[nodiscard]] inline long strtol(const char* nptr, char** endptr, int base) noexcept {
    const char* s;
    unsigned long acc;
    char c;
    unsigned long cutoff;
    int neg;
    int any;
    int cutlim;

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
    if ((base == 0 || base == 2) && c == '0' && (*s == 'b' || *s == 'B') &&
        (s[1] >= '0' && s[1] <= '1')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    acc = any = 0;
    if (base < 2 || base > 36) {
        goto noconv;
    }

    cutoff = neg ? static_cast<unsigned long>(-(LONG_MIN + LONG_MAX)) + LONG_MAX
                 : LONG_MAX;
    cutlim = static_cast<int>(cutoff % static_cast<unsigned long>(base));
    cutoff /= static_cast<unsigned long>(base);

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
            (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= static_cast<unsigned long>(base);
            acc += static_cast<unsigned long>(c);
        }
    }

    if (any < 0) {
        acc = neg ? static_cast<unsigned long>(LONG_MIN) : LONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv:
        errno = EINVAL;
    } else if (neg) {
        acc = static_cast<unsigned long>(0u - acc);
    }

    if (endptr != nullptr) {
        *endptr = const_cast<char*>(any ? s - 1 : nptr);
    }
    return static_cast<long>(acc);
}

[[nodiscard]] inline int atoi(const char* str) noexcept {
    return static_cast<int>(strtol(str, nullptr, 10));
}

[[nodiscard]] inline long atol(const char* str) noexcept {
    return strtol(str, nullptr, 10);
}

[[nodiscard]] inline unsigned long strtoul(const char* nptr, char** endptr,
                                           int base) noexcept {
    const char* s;
    unsigned long acc;
    char c;
    unsigned long cutoff;
    int neg;
    int any;
    int cutlim;

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
    if ((base == 0 || base == 2) && c == '0' && (*s == 'b' || *s == 'B') &&
        (s[1] >= '0' && s[1] <= '1')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    acc = any = 0;
    if (base < 2 || base > 36) {
        goto noconv_ul;
    }

    cutoff = ULONG_MAX / static_cast<unsigned long>(base);
    cutlim = static_cast<int>(ULONG_MAX % static_cast<unsigned long>(base));

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
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= static_cast<unsigned long>(base);
            acc += static_cast<unsigned long>(c);
        }
    }

    if (any < 0) {
        acc = ULONG_MAX;
        errno = ERANGE;
    } else if (!any) {
noconv_ul:
        errno = EINVAL;
    } else if (neg) {
        acc = static_cast<unsigned long>(-static_cast<long>(acc));
    }

    if (endptr != nullptr) {
        *endptr = const_cast<char*>(any ? s - 1 : nptr);
    }
    return acc;
}

[[nodiscard]] inline int abs(int j) noexcept {
    return j < 0 ? -j : j;
}

[[nodiscard]] inline long labs(long j) noexcept {
    return j < 0 ? -j : j;
}

} // namespace pbsd::userland::libc
