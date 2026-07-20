module;

#include <cerrno>
#include <climits>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.userland.util.numbers;

import pbsd.core;

/// humanize_number(3) + expand_number(3) from hbsd/src/lib/libutil/.
export namespace pbsd::userland::util {

inline constexpr int kMaxHumanizeScale = 6;

inline constexpr int HN_DECIMAL       = 0x01;
inline constexpr int HN_NOSPACE       = 0x02;
inline constexpr int HN_B             = 0x04;
inline constexpr int HN_DIVISOR_1000  = 0x08;
inline constexpr int HN_IEC_PREFIXES  = 0x10;
inline constexpr int HN_GETSCALE      = 0x10;
inline constexpr int HN_AUTOSCALE     = 0x20;

[[nodiscard]] inline int humanize_number(char* buf, unsigned len,
                                         long long quotient,
                                         const char* suffix, int scale,
                                         int flags) noexcept {
    const char* prefixes;
    const char* sep;
    int i, r, remainder, s1, s2, sign;
    int divisordeccut;
    long long divisor, max;
    unsigned baselen;

    if (len > 0) {
        buf[0] = '\0';
    }
    if (buf == nullptr || suffix == nullptr) {
        return -1;
    }
    if (scale < 0) {
        return -1;
    }
    if (scale > kMaxHumanizeScale &&
        ((scale & ~(HN_AUTOSCALE | HN_GETSCALE)) != 0)) {
        return -1;
    }
    if ((flags & HN_DIVISOR_1000) != 0 && (flags & HN_IEC_PREFIXES) != 0) {
        return -1;
    }

    remainder = 0;

    if ((flags & HN_IEC_PREFIXES) != 0) {
        baselen = 2;
        divisor = 1024;
        divisordeccut = 973;
        prefixes = ((flags & HN_B) != 0)
            ? "B\0\0Ki\0Mi\0Gi\0Ti\0Pi\0Ei"
            : "\0\0\0Ki\0Mi\0Gi\0Ti\0Pi\0Ei";
    } else {
        baselen = 1;
        if ((flags & HN_DIVISOR_1000) != 0) {
            divisor = 1000;
            divisordeccut = 950;
            prefixes = ((flags & HN_B) != 0)
                ? "B\0\0k\0\0M\0\0G\0\0T\0\0P\0\0E"
                : "\0\0\0k\0\0M\0\0G\0\0T\0\0P\0\0E";
        } else {
            divisor = 1024;
            divisordeccut = 973;
            prefixes = ((flags & HN_B) != 0)
                ? "B\0\0K\0\0M\0\0G\0\0T\0\0P\0\0E"
                : "\0\0\0K\0\0M\0\0G\0\0T\0\0P\0\0E";
        }
    }

#define SCALE2PREFIX(sc) (&prefixes[(sc)*3])

    if (quotient < 0) {
        sign = -1;
        quotient = -quotient;
        baselen += 2;
    } else {
        sign = 1;
        baselen += 1;
    }
    if ((flags & HN_NOSPACE) != 0) {
        sep = "";
    } else {
        sep = " ";
        ++baselen;
    }
    baselen += static_cast<unsigned>(std::strlen(suffix));

    if (len < baselen + 1) {
        return -1;
    }

    if ((scale & (HN_AUTOSCALE | HN_GETSCALE)) != 0) {
        for (max = 1, i = static_cast<int>(len - baselen); i-- > 0;) {
            max *= 10;
        }
        for (i = 0;
             (quotient >= max ||
              (quotient == max - 1 &&
               (remainder >= divisordeccut || remainder >= divisor / 2))) &&
             i < kMaxHumanizeScale;
             ++i) {
            remainder = static_cast<int>(quotient % divisor);
            quotient /= divisor;
        }
        if ((scale & HN_GETSCALE) != 0) {
            return i;
        }
    } else {
        for (i = 0; i < scale && i < kMaxHumanizeScale; ++i) {
            remainder = static_cast<int>(quotient % divisor);
            quotient /= divisor;
        }
    }

    if (((quotient == 9 && remainder < divisordeccut) || quotient < 9) &&
        i > 0 && (flags & HN_DECIMAL) != 0) {
        s1 = static_cast<int>(quotient) +
             ((remainder * 10 + divisor / 2) / divisor / 10);
        s2 = ((remainder * 10 + divisor / 2) / divisor) % 10;
        r = std::snprintf(buf, len, "%d%s%d%s%s%s", sign * s1,
                          std::localeconv()->decimal_point, s2, sep,
                          SCALE2PREFIX(i), suffix);
    } else {
        r = std::snprintf(buf, len, "%lld%s%s%s",
                          sign * (quotient + (remainder + divisor / 2) / divisor),
                          sep, SCALE2PREFIX(i), suffix);
    }

#undef SCALE2PREFIX
    return r;
}

namespace detail {

[[nodiscard]] inline int expand_impl(const char* buf, unsigned long long* num,
                                   bool* neg) noexcept {
    char* endptr = nullptr;
    unsigned long long number = 0;
    unsigned shift = 0;
    int serrno = 0;

    while (*buf != '\0' &&
           (*buf == ' ' || *buf == '\t' || *buf == '\n' || *buf == '\r')) {
        ++buf;
    }
    if (*buf == '-') {
        *neg = true;
        ++buf;
    } else {
        *neg = false;
        if (*buf == '+') {
            ++buf;
        }
    }

    if (*buf < '0' || *buf > '9') {
        return -1;
    }

    serrno = errno;
    errno = 0;
    number = std::strtoull(buf, &endptr, 0);
    if (errno != 0) {
        return -1;
    }
    errno = serrno;

    if (endptr != nullptr) {
        switch (*endptr | 0x20) {
        case 'e':
            shift = 60;
            ++endptr;
            break;
        case 'p':
            shift = 50;
            ++endptr;
            break;
        case 't':
            shift = 40;
            ++endptr;
            break;
        case 'g':
            shift = 30;
            ++endptr;
            break;
        case 'm':
            shift = 20;
            ++endptr;
            break;
        case 'k':
            shift = 10;
            ++endptr;
            break;
        default:
            shift = 0;
            break;
        }
        if ((*endptr | 0x20) == 'b') {
            ++endptr;
        }
        if (*endptr != '\0') {
            return -1;
        }
    }

    if ((number << shift) >> shift != number) {
        return -1;
    }
    number <<= shift;

    *num = number;
    return 0;
}

} // namespace detail

[[nodiscard]] inline int expand_number(const char* buf, long long* num) noexcept {
    unsigned long long number = 0;
    bool neg = false;

    if (detail::expand_impl(buf, &number, &neg) != 0) {
        return -1;
    }

    if (neg) {
        if (number > 0x8000000000000000ULL) {
            return -1;
        }
        *num = -static_cast<long long>(number);
    } else {
        if (number > static_cast<unsigned long long>(LLONG_MAX)) {
            return -1;
        }
        *num = static_cast<long long>(number);
    }
    return 0;
}

[[nodiscard]] inline int expand_unsigned(const char* buf,
                                         unsigned long long* num) noexcept {
    unsigned long long number = 0;
    bool neg = false;

    if (detail::expand_impl(buf, &number, &neg) != 0) {
        return -1;
    }
    if (neg && number > 0) {
        return -1;
    }
    *num = number;
    return 0;
}

} // namespace pbsd::userland::util
