module;

#include <cctype>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#endif

export module pbsd.userland.util.host;

/// trimdomain(3) + realhostname(3) constants from hbsd/src/lib/libutil/.
export namespace pbsd::userland::util {

inline constexpr int HOSTNAME_FOUND          = 0;
inline constexpr int HOSTNAME_INCORRECTNAME  = 1;
inline constexpr int HOSTNAME_INVALIDADDR    = 2;
inline constexpr int HOSTNAME_INVALIDNAME    = 3;

inline constexpr unsigned kMaxHostNameLen = 256;

namespace detail {

[[nodiscard]] inline bool is_display(const char* disp) noexcept {
    unsigned w = 0;
    bool res = false;

    while (disp[w] >= '0' && disp[w] <= '9') {
        ++w;
    }
    if (w > 0) {
        if (disp[w] == '\0') {
            res = true;
        } else if (disp[w] == '.') {
            disp += w + 1;
            w = 0;
            while (disp[w] >= '0' && disp[w] <= '9') {
                ++w;
            }
            if (w > 0 && disp[w] == '\0') {
                res = true;
            }
        }
    }
    return res;
}

[[nodiscard]] inline int strncasecmp_local(const char* a, const char* b,
                                           unsigned n) noexcept {
    for (unsigned i = 0; i < n; ++i) {
        const unsigned ca = static_cast<unsigned char>(a[i]);
        const unsigned cb = static_cast<unsigned char>(b[i]);
        if (ca == '\0' || cb == '\0') {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        const int da = std::tolower(ca);
        const int db = std::tolower(cb);
        if (da != db) {
            return da - db;
        }
    }
    return 0;
}

[[nodiscard]] inline unsigned strnlen_local(const char* s, unsigned n) noexcept {
    unsigned i = 0;
    while (i < n && s[i] != '\0') {
        ++i;
    }
    return i;
}

} // namespace detail

/// Trim current domain from fullhost when result fits hostsize (DISPLAY-aware).
inline void trimdomain(char* fullhost, unsigned hostsize) noexcept {
    static unsigned dlen = 0;
    static bool first = true;
    static char domain[kMaxHostNameLen]{};

    if (first) {
        first = false;
#ifndef _WIN32
        if (gethostname(domain, sizeof(domain) - 1) == 0) {
            char* dot = std::strchr(domain, '.');
            if (dot != nullptr) {
                std::memmove(domain, dot + 1, std::strlen(dot + 1) + 1);
            }
        } else
#endif
        {
            domain[0] = '\0';
        }
        dlen = static_cast<unsigned>(std::strlen(domain));
    }

    if (domain[0] == '\0') {
        return;
    }

    hostsize = detail::strnlen_local(fullhost, hostsize);

    char* s = fullhost;
    char* end = s + hostsize + 1;
    s = static_cast<char*>(std::memchr(s, '.', static_cast<unsigned>(end - s)));
    if (s != nullptr) {
        if (detail::strncasecmp_local(s + 1, domain, dlen) == 0) {
            if (s[dlen + 1] == '\0') {
                *s = '\0';
            } else if (s[dlen + 1] == ':' &&
                       detail::is_display(s + dlen + 2)) {
                const unsigned len = static_cast<unsigned>(std::strlen(s + dlen + 1));
                if (len < static_cast<unsigned>(end - s)) {
                    std::memmove(s, s + dlen + 1, len + 1);
                }
            }
        }
    }
}

} // namespace pbsd::userland::util
