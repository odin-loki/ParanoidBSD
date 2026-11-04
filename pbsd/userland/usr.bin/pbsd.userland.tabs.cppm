module;
#include <cstddef>

export module pbsd.userland.tabs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tabs/tabs.c — tab stop parsing (logic-only).
export namespace pbsd::userland::usr_bin::tabs {

inline constexpr int kMaxStops = 20;
inline constexpr int kDefIncrement = 8;

struct TabFormat {
    const char* name;
    long stops[kMaxStops];
};

inline constexpr TabFormat kFormats[] = {
    {"a", {1, 10, 16, 36, 72, 0}},
    {"a2", {1, 10, 16, 40, 72, 0}},
    {"c", {1, 8, 12, 16, 20, 55, 0}},
    {"c2", {1, 6, 10, 14, 49, 0}},
    {"c3", {1, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 67, 0}},
    {"f", {1, 7, 11, 15, 19, 23, 0}},
    {"p", {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 0}},
    {"s", {1, 10, 55, 0}},
    {"u", {1, 12, 20, 44, 0}},
};

struct Options {
    long stops[kMaxStops]{};
    int stop_count{-1};
    long increment{kDefIncrement};
    long margin{0};
    bool has_margin{false};
};

[[nodiscard]] inline int format_count() noexcept {
    return static_cast<int>(sizeof(kFormats) / sizeof(kFormats[0]));
}

[[nodiscard]] inline Result<int> find_format(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    for (int i = 0; i < format_count(); ++i) {
        if (hosted::cstrcmp(kFormats[i].name, name) == 0) {
            return result_ok(i);
        }
    }
    return result_err<int>(Status::Invalid);
}

[[nodiscard]] inline Result<Options> load_format(int index) noexcept {
    if (index < 0 || index >= format_count()) {
        return result_err<Options>(Status::Invalid);
    }
    Options opt{};
    opt.stop_count = 0;
    for (int j = 0; j < kMaxStops && kFormats[index].stops[j] != 0; ++j) {
        opt.stops[opt.stop_count++] = kFormats[index].stops[j];
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<long> parse_positive(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<long>(Status::Invalid);
    }
    long val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<long>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_tab_list(const char* spec, Options& opt) noexcept {
    if (spec == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    opt.stop_count = 0;
    long last = 0;
    const char* tok_start = spec;
    while (*tok_start != '\0') {
        const char* tok_end = tok_start;
        while (*tok_end != '\0' && *tok_end != ',') {
            ++tok_end;
        }
        if (opt.stop_count >= kMaxStops) {
            return result_err<Options>(Status::Invalid);
        }
        bool relative = (*tok_start == '+');
        const char* digits = relative ? tok_start + 1 : tok_start;
        auto parsed = parse_positive(digits);
        if (parsed.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
        long stop = parsed.value;
        if (relative) {
            if (tok_start == spec) {
                return result_err<Options>(Status::Invalid);
            }
            stop += last;
        }
        if (last > stop) {
            return result_err<Options>(Status::Invalid);
        }
        last = opt.stops[opt.stop_count++] = stop;
        tok_start = (*tok_end == ',') ? tok_end + 1 : tok_end;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::tabs
