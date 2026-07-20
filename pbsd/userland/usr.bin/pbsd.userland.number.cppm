module;
#include <cstddef>

export module pbsd.userland.number;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/number/number.c — number-to-words helpers (logic-only).
export namespace pbsd::userland::usr_bin::number {

inline constexpr int kMaxDigits = 65;

struct Options {
    bool line_mode{false};
};

inline constexpr const char* kOnes[] = {
    "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen",
    "seventeen", "eighteen", "nineteen",
};

inline constexpr const char* kTens[] = {
    "", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy",
    "eighty", "ninety",
};

inline constexpr const char* kScales[] = {
    "hundred", "thousand", "million", "billion", "trillion", "quadrillion",
    "quintillion", "sextillion", "septillion", "octillion", "nonillion",
    "decillion", "undecillion", "duodecillion", "tredecillion",
    "quattuordecillion", "quindecillion", "sexdecillion", "septendecillion",
    "octodecillion", "novemdecillion", "vigintillion",
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (hosted::cstrcmp(argv[i], "-l") == 0) {
            opt.line_mode = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool is_digit_char(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] inline bool is_number_char(char c) noexcept {
    return is_digit_char(c) || c == '.' || c == '-';
}

[[nodiscard]] inline int digit_count(const char* s) noexcept {
    if (s == nullptr) {
        return 0;
    }
    int count = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (is_digit_char(*p)) {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] inline bool valid_number_line(const char* line) noexcept {
    if (line == nullptr) {
        return false;
    }
    bool seen_digit = false;
    bool seen_dot = false;
    for (const char* p = line; *p != '\0' && *p != '\n'; ++p) {
        if (*p == ' ' || *p == '\t') {
            continue;
        }
        if (is_digit_char(*p)) {
            seen_digit = true;
            continue;
        }
        if (*p == '.' && !seen_dot) {
            seen_dot = true;
            continue;
        }
        if (*p == '-' && p == line) {
            continue;
        }
        return false;
    }
    return seen_digit && digit_count(line) <= kMaxDigits;
}

[[nodiscard]] inline int two_digit_value(const char* p) noexcept {
    if (p == nullptr) {
        return 0;
    }
    return (p[0] - '0') * 10 + (p[1] - '0');
}

} // namespace pbsd::userland::usr_bin::number
