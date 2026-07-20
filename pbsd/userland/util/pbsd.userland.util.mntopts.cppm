module;

#include <cctype>
#include <cstring>

export module pbsd.userland.util.mntopts;

import pbsd.core;

/// getmntopts(3) option parsing from hbsd/src/lib/libutil/mntopts.c (logic-only).
export namespace pbsd::userland::util::mntopts {

struct MountOption {
    const char* name{nullptr};
    int flag{0};
    bool alt_loc{false};
    bool inverse{false};
};

struct ParseResult {
    int primary_flags{0};
    int alt_flags{0};
    bool silent{false};
};

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

[[nodiscard]] inline bool option_matches(const char* opt, const char* pattern) noexcept {
    if (opt == nullptr || pattern == nullptr) {
        return false;
    }
    const unsigned len = static_cast<unsigned>(std::strlen(pattern));
    if (strncasecmp_local(opt, pattern, len) != 0) {
        return false;
    }
    return opt[len] == '\0' || opt[len] == '=';
}

[[nodiscard]] inline Status parse_options(const char* options, const MountOption* table,
                                          ParseResult& out) noexcept {
    if (options == nullptr || table == nullptr) {
        return Status::Invalid;
    }

    char buf[512]{};
    const std::size_t n = std::strlen(options);
    if (n >= sizeof(buf)) {
        return Status::Invalid;
    }
    std::memcpy(buf, options, n + 1);

    char* cursor = buf;
    while (cursor != nullptr && *cursor != '\0') {
        char* comma = std::strchr(cursor, ',');
        if (comma != nullptr) {
            *comma = '\0';
        }

        bool negative = false;
        char* opt = cursor;
        if (opt[0] == 'n' && opt[1] == 'o') {
            negative = true;
            opt += 2;
        }

        char* eq = std::strchr(opt, '=');
        if (eq != nullptr) {
            *eq = '\0';
        }

        bool matched = false;
        for (const MountOption* m = table; m->name != nullptr; ++m) {
            if (!option_matches(opt, m->name)) {
                continue;
            }
            matched = true;
            int* target = m->alt_loc ? &out.alt_flags : &out.primary_flags;
            if (!negative) {
                *target |= m->flag;
            } else {
                *target &= ~m->flag;
            }
            break;
        }

        if (!matched && !out.silent) {
            return Status::NotFound;
        }

        cursor = comma != nullptr ? comma + 1 : nullptr;
    }

    return Status::Ok;
}

} // namespace pbsd::userland::util::mntopts
