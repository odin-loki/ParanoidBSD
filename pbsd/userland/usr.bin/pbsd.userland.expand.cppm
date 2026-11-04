module;
#include <cstddef>

export module pbsd.userland.expand;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/expand/expand.c — tabstop helpers (logic-only).
export namespace pbsd::userland::usr_bin::expand {

inline constexpr int kMaxStops = 100;
inline constexpr int kDefTab = 8;

struct Options {
    int stops[kMaxStops]{};
    int stop_count{0};
    bool all_tabs{false};
};

[[nodiscard]] inline Result<Options> parse_tabstops(const char* spec, Options& opt) noexcept {
    if (spec == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    opt.stop_count = 0;
    const char* p = spec;
    while (*p != '\0') {
        int val = 0;
        if (*p < '0' || *p > '9') {
            return result_err<Options>(Status::Invalid);
        }
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            ++p;
        }
        if (val <= 0 || opt.stop_count >= kMaxStops) {
            return result_err<Options>(Status::Invalid);
        }
        opt.stops[opt.stop_count++] = val;
        if (*p == ',') {
            ++p;
        } else if (*p != '\0') {
            return result_err<Options>(Status::Invalid);
        }
    }
    return result_ok(opt);
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
        if (hosted::cstrcmp(arg, "-t") == 0) {
            opt.all_tabs = true;
            continue;
        }
        if (arg[1] == 't' && arg[2] != '\0') {
            if (parse_tabstops(arg + 2, opt).status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (arg[1] == 't' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            if (parse_tabstops(argv[++i], opt).status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (opt.stop_count == 0 && !opt.all_tabs) {
        opt.stops[0] = kDefTab;
        opt.stop_count = 1;
    }
    return result_ok(opt);
}

[[nodiscard]] inline int next_tab(int column, const Options& opt) noexcept {
    if (opt.stop_count <= 0) {
        return column + (kDefTab - column % kDefTab);
    }
    for (int i = 0; i < opt.stop_count; ++i) {
        if (opt.stops[i] > column) {
            return opt.stops[i];
        }
    }
    return column + kDefTab;
}

} // namespace pbsd::userland::usr_bin::expand
