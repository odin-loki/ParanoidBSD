module;
#include <cstddef>

export module pbsd.userland.unexpand;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/unexpand/unexpand.c — tabify helpers (logic-only).
export namespace pbsd::userland::usr_bin::unexpand {

inline constexpr int kMaxStops = 100;
inline constexpr int kDefTab = 8;

struct Options {
    int stops[kMaxStops]{};
    int stop_count{1};
    bool all_spaces{false};
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
    opt.stops[0] = kDefTab;
    opt.stop_count = 1;
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-a") == 0) {
            opt.all_spaces = true;
            continue;
        }
        if (arg[1] == 't' && arg[2] != '\0') {
            if (parse_tabstops(arg + 2, opt).status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.all_spaces = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-t") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            if (parse_tabstops(argv[++i], opt).status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.all_spaces = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline int advance_tab(int column, const Options& opt) noexcept {
    if (opt.stop_count <= 0) {
        return (1 + column / kDefTab) * kDefTab;
    }
    for (int i = 0; i < opt.stop_count; ++i) {
        if (opt.stops[i] > column) {
            return opt.stops[i];
        }
    }
    return column + kDefTab;
}

[[nodiscard]] inline int last_stop(const Options& opt) noexcept {
    if (opt.stop_count <= 0) {
        return kDefTab;
    }
    return opt.stops[opt.stop_count - 1];
}

} // namespace pbsd::userland::usr_bin::unexpand
