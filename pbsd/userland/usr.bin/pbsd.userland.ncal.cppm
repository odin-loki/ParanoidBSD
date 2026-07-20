module;
#include <cstddef>

export module pbsd.userland.ncal;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ncal/ncal.c — calendar grid helpers (logic-only).
export namespace pbsd::userland::usr_bin::ncal {

struct Options {
    bool julian{false};
    bool week_numbers{false};
    bool month_only{false};
    int month{0};
    int year{0};
};

[[nodiscard]] inline bool is_leap_year(int year) noexcept {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

[[nodiscard]] inline int days_in_month(int month, int year) noexcept {
    static constexpr int kMonthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return kMonthDays[month - 1];
}

[[nodiscard]] inline int weekday_jan1(int year) noexcept {
    int y = year;
    int m = 1;
    if (m < 3) {
        m += 12;
        y -= 1;
    }
    const int k = y % 100;
    const int j = y / 100;
    const int h = (1 + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (h + 6) % 7;
}

[[nodiscard]] inline bool valid_month(int month) noexcept {
    return month >= 1 && month <= 12;
}

[[nodiscard]] inline bool valid_year(int year) noexcept {
    return year >= 1 && year <= 9999;
}

[[nodiscard]] inline Result<int> day_column(int year, int month, int day) noexcept {
    if (!valid_year(year) || !valid_month(month)) {
        return result_err<int>(Status::Invalid);
    }
    if (day < 1 || day > days_in_month(month, year)) {
        return result_err<int>(Status::Invalid);
    }
    int offset = weekday_jan1(year);
    for (int m = 1; m < month; ++m) {
        offset = (offset + days_in_month(m, year)) % 7;
    }
    return result_ok((offset + day - 1) % 7);
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
        if (hosted::cstrcmp(arg, "-J") == 0) {
            opt.julian = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-w") == 0) {
            opt.week_numbers = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-m") == 0) {
            opt.month_only = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i == 1) {
        const char* year_str = argv[i];
        int val = 0;
        for (const char* p = year_str; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<Options>(Status::Invalid);
            }
            val = val * 10 + (*p - '0');
        }
        if (!valid_year(val)) {
            return result_err<Options>(Status::Invalid);
        }
        opt.year = val;
    } else if (argc - i == 2) {
        const char* month_str = argv[i];
        const char* year_str = argv[i + 1];
        int month = 0;
        int year = 0;
        for (const char* p = month_str; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<Options>(Status::Invalid);
            }
            month = month * 10 + (*p - '0');
        }
        for (const char* p = year_str; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<Options>(Status::Invalid);
            }
            year = year * 10 + (*p - '0');
        }
        if (!valid_month(month) || !valid_year(year)) {
            return result_err<Options>(Status::Invalid);
        }
        opt.month = month;
        opt.year = year;
    } else if (argc - i > 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::ncal
