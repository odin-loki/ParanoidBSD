module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.date;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/date/date.c + vary.c — parsing and adjustment helpers.
export namespace pbsd::userland::bin::date {

inline constexpr int kTmYearBase = 1900;

struct Iso8601Format {
    const char* refname;
    const char* format_string;
};

inline constexpr Iso8601Format kIso8601Formats[] = {
    {"date", "%Y-%m-%d"},
    {"hours", "T%H"},
    {"minutes", ":%M"},
    {"seconds", ":%S"},
    {"ns", ",%N"},
};

struct Options {
    bool iso8601{false};
    bool rfc2822{false};
    bool utc{false};
    bool no_set{false}; // -j
    const char* format{nullptr};
    const char* iso8601_part{nullptr};
    const char* reference_file{nullptr};
    const char* output_zone{nullptr};
};

struct VaryEntry {
    const char* arg{nullptr};
    const VaryEntry* next{nullptr};
};

struct ParsedVary {
    char op{'\0'}; // +, -, or set
    char unit{'\0'}; // S,M,H,d,w,m,y
    std::int64_t value{0};
    bool is_text{false};
};

struct CompactDateFields {
    int sec{0};
    int min{0};
    int hour{0};
    int mday{0};
    int mon{0}; // 0-11 after parse
    int year{0}; // years since 1900
};

[[nodiscard]] inline bool names_equal(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b) == 0;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == 'I') {
            if (opt.rfc2822) {
                return result_err<Options>(Status::Invalid);
            }
            opt.iso8601 = true;
            if (flag[2] == '\0') {
                opt.iso8601_part = "date";
            } else if (flag[2] == '=' && flag[3] != '\0') {
                opt.iso8601_part = flag + 3;
            } else {
                return result_err<Options>(Status::Invalid);
            }
        } else if (flag[1] == 'R' && flag[2] == '\0') {
            if (opt.iso8601) {
                return result_err<Options>(Status::Invalid);
            }
            opt.rfc2822 = true;
        } else if (flag[1] == 'j' && flag[2] == '\0') {
            opt.no_set = true;
        } else if (flag[1] == 'u' && flag[2] == '\0') {
            opt.utc = true;
        } else if (flag[1] == 'f' && flag[2] != '\0') {
            opt.format = flag + 2;
        } else if (flag[1] == 'f' && flag[2] == '\0') {
            ++i;
            if (i >= argc || argv[i] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            opt.format = argv[i];
        } else if (flag[1] == 'r' && flag[2] != '\0') {
            opt.reference_file = flag + 2;
        } else if (flag[1] == 'r' && flag[2] == '\0') {
            ++i;
            if (i >= argc || argv[i] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            opt.reference_file = argv[i];
        } else if (flag[1] == 'z' && flag[2] != '\0') {
            opt.output_zone = flag + 2;
        } else if (flag[1] == 'z' && flag[2] == '\0') {
            ++i;
            if (i >= argc || argv[i] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            opt.output_zone = argv[i];
        } else if (flag[1] == 'n' && flag[2] == '\0') {
            // accepted no-op
        } else if (flag[1] == 'v' && flag[2] != '\0') {
            // vary list handled separately
        } else {
            return result_err<Options>(Status::Invalid);
        }
        ++i;
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<const Iso8601Format*> lookup_iso8601_part(
    const char* name) noexcept {
    if (name == nullptr) {
        return result_err<const Iso8601Format*>(Status::Invalid);
    }
    for (const auto& fmt : kIso8601Formats) {
        if (names_equal(fmt.refname, name)) {
            return result_ok(&fmt);
        }
    }
    return result_err<const Iso8601Format*>(Status::NotFound);
}

[[nodiscard]] inline bool is_digit_string(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return false;
    }
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline Result<ParsedVary> parse_vary_spec(const char* arg) noexcept {
    ParsedVary pv{};
    if (arg == nullptr || arg[0] == '\0') {
        return result_err<ParsedVary>(Status::Invalid);
    }
    const char* p = arg;
    if (*p == '+' || *p == '-') {
        pv.op = *p;
        ++p;
    }
    const std::size_t len = hosted::cstrlen(p);
    if (len < 2) {
        return result_err<ParsedVary>(Status::Invalid);
    }
    pv.unit = p[len - 1];
    if (is_digit_string(p)) {
        long val = 0;
        for (std::size_t i = 0; i + 1 < len; ++i) {
            val = val * 10 + (p[i] - '0');
        }
        pv.value = val;
        pv.is_text = false;
        return result_ok(pv);
    }
    pv.is_text = true;
    return result_ok(pv);
}

[[nodiscard]] inline int atoi2(const char*& s) noexcept {
    const int hi = s[0] - '0';
    const int lo = s[1] - '0';
    s += 2;
    return hi * 10 + lo;
}

/// Parse compact date string from setthetime() default format.
[[nodiscard]] inline Result<CompactDateFields> parse_compact_datetime(
    const char* p) noexcept {
    CompactDateFields f{};
    if (p == nullptr) {
        return result_err<CompactDateFields>(Status::Invalid);
    }

    const char* dot = nullptr;
    for (const char* t = p; *t != '\0'; ++t) {
        if (*t < '0' || *t > '9') {
            if (*t == '.' && dot == nullptr) {
                dot = t;
                continue;
            }
            return result_err<CompactDateFields>(Status::Invalid);
        }
    }

    if (dot != nullptr) {
        const char* d = dot + 1;
        if (hosted::cstrlen(d) != 2) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
        f.sec = atoi2(d);
        if (f.sec > 61) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
    }

    const std::size_t body_len = hosted::cstrlen(p) - ((dot != nullptr) ? 3 : 0);
    const char* t = p;
    int century = 0;
    switch (body_len) {
    case 12:
        f.year = atoi2(t) * 100 - kTmYearBase;
        century = 1;
        [[fallthrough]];
    case 10:
        if (century) {
            f.year += atoi2(t);
        } else {
            f.year = atoi2(t);
            if (f.year < 69) {
                f.year += 2000 - kTmYearBase;
            } else {
                f.year += 1900 - kTmYearBase;
            }
        }
        [[fallthrough]];
    case 8:
        f.mon = atoi2(t);
        if (f.mon > 12 || f.mon == 0) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
        --f.mon;
        [[fallthrough]];
    case 6:
        f.mday = atoi2(t);
        if (f.mday > 31 || f.mday == 0) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
        [[fallthrough]];
    case 4:
        f.hour = atoi2(t);
        if (f.hour > 23) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
        [[fallthrough]];
    case 2:
        f.min = atoi2(t);
        if (f.min > 59) {
            return result_err<CompactDateFields>(Status::Invalid);
        }
        break;
    default:
        return result_err<CompactDateFields>(Status::Invalid);
    }
    return result_ok(f);
}

[[nodiscard]] inline int days_in_month(int year_since_1900, int mon) noexcept {
    static const int mdays[] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (mon < 0 || mon > 11) {
        return 0;
    }
    if (mon == 1) {
        const int year = year_since_1900 + kTmYearBase;
        if (year % 400 == 0) {
            return 29;
        }
        if (year % 100 == 0) {
            return 28;
        }
        if (year % 4 == 0) {
            return 29;
        }
        return 28;
    }
    return mdays[mon];
}

[[nodiscard]] inline const char* default_format_string(const Options& opt) noexcept {
    if (opt.rfc2822) {
        return "%a, %d %b %Y %T %z";
    }
    return "%+";
}

} // namespace pbsd::userland::bin::date
