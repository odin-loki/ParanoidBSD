module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.primes;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/primes/primes.c — range and sieve helpers (logic-only).
export namespace pbsd::userland::usr_bin::primes {

using ubig = std::uint64_t;

inline constexpr ubig kBig = ~ubig{0};
inline constexpr ubig kSieveMax = 4295098368ULL;
inline constexpr int kTabSize = 256 * 1024;

struct Options {
    bool hex_output{false};
    ubig start{0};
    ubig stop{kBig};
};

[[nodiscard]] inline bool has_leading_minus(const char* s) noexcept {
    return s != nullptr && s[0] == '-';
}

[[nodiscard]] inline Result<ubig> parse_ubig(const char* s) noexcept {
    if (s == nullptr || *s == '\0' || has_leading_minus(s)) {
        return result_err<ubig>(Status::Invalid);
    }
    ubig val = 0;
    int base = 10;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s += 2;
    }
    for (; *s != '\0'; ++s) {
        int digit = -1;
        if (*s >= '0' && *s <= '9') {
            digit = *s - '0';
        } else if (base == 16 && *s >= 'a' && *s <= 'f') {
            digit = 10 + (*s - 'a');
        } else if (base == 16 && *s >= 'A' && *s <= 'F') {
            digit = 10 + (*s - 'A');
        } else {
            return result_err<ubig>(Status::Invalid);
        }
        val = val * static_cast<ubig>(base) + static_cast<ubig>(digit);
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-h") == 0) {
            opt.hex_output = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    const int rem = argc - i;
    if (rem == 2) {
        auto start = parse_ubig(argv[i]);
        auto stop = parse_ubig(argv[i + 1]);
        if (start.status != Status::Ok || stop.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
        opt.start = start.value;
        opt.stop = stop.value;
    } else if (rem == 1) {
        auto start = parse_ubig(argv[i]);
        if (start.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
        opt.start = start.value;
    } else if (rem > 2) {
        return result_err<Options>(Status::Invalid);
    }
    if (opt.start > opt.stop) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool is_odd_candidate(ubig n) noexcept {
    return (n & 1U) != 0U;
}

[[nodiscard]] inline bool trial_divide_small(ubig n) noexcept {
    if (n < 2) {
        return false;
    }
    if (n == 2 || n == 3) {
        return true;
    }
    if ((n % 2U) == 0U || (n % 3U) == 0U) {
        return false;
    }
    for (ubig d = 5; d * d <= n; d += 4) {
        if ((n % d) == 0U) {
            return false;
        }
        d += 2;
        if ((n % d) == 0U) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::userland::usr_bin::primes
