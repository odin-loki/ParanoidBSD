module;
#include <cstddef>

export module pbsd.userland.cut;

export import pbsd.core;

/// Port helpers from hbsd/src/usr.bin/cut/cut.c — field/byte list parse.
export namespace pbsd::userland::usr_bin::cut {

enum class Mode : unsigned char { Bytes, Chars, Fields };

struct Options {
    Mode mode{Mode::Fields};
    char delim{'\t'};
    bool suppress_no_delim{false}; // -s
    // Selected 1-based indices packed as bitmask for columns 1..64 (scaffold)
    unsigned long long select_mask{0};
};

[[nodiscard]] inline Result<unsigned long long> parse_list(const char* list) noexcept {
    if (list == nullptr || list[0] == '\0') {
        return result_err<unsigned long long>(Status::Invalid);
    }
    unsigned long long mask = 0;
    const char* p = list;
    while (*p) {
        if (*p < '0' || *p > '9') {
            return result_err<unsigned long long>(Status::Invalid);
        }
        unsigned n = 0;
        while (*p >= '0' && *p <= '9') {
            n = n * 10 + static_cast<unsigned>(*p - '0');
            ++p;
        }
        if (n == 0 || n > 64) {
            return result_err<unsigned long long>(Status::Invalid);
        }
        unsigned long long bit = 1ull << (n - 1);
        if (*p == '-') {
            ++p;
            if (*p < '0' || *p > '9') {
                // open-ended N- → through 64
                for (unsigned k = n; k <= 64; ++k) {
                    mask |= (1ull << (k - 1));
                }
            } else {
                unsigned m = 0;
                while (*p >= '0' && *p <= '9') {
                    m = m * 10 + static_cast<unsigned>(*p - '0');
                    ++p;
                }
                if (m < n || m > 64) {
                    return result_err<unsigned long long>(Status::Invalid);
                }
                for (unsigned k = n; k <= m; ++k) {
                    mask |= (1ull << (k - 1));
                }
            }
        } else {
            mask |= bit;
        }
        if (*p == ',') {
            ++p;
        } else if (*p != '\0') {
            return result_err<unsigned long long>(Status::Invalid);
        }
    }
    return result_ok(mask);
}

[[nodiscard]] inline bool selected(unsigned long long mask, unsigned one_based) noexcept {
    if (one_based == 0 || one_based > 64) {
        return false;
    }
    return (mask & (1ull << (one_based - 1))) != 0;
}

} // namespace pbsd::userland::usr_bin::cut
