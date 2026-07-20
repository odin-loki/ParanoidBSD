module;
#include <cstddef>

export module pbsd.userland.nice;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/nice/nice.c — niceness parsing (logic-only).
export namespace pbsd::userland::usr_bin::nice {

inline constexpr long kDefNice = 10;

struct Options {
    long increment{kDefNice};
};

[[nodiscard]] inline Result<long> parse_nice_value(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<long>(Status::Invalid);
    }
    const char* p = s;
    bool negative = false;
    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        ++p;
    }
    if (*p == '\0') {
        return result_err<long>(Status::Invalid);
    }
    long val = 0;
    for (; *p >= '0' && *p <= '9'; ++p) {
        val = val * 10 + (*p - '0');
    }
    if (*p != '\0') {
        return result_err<long>(Status::Invalid);
    }
    if (negative) {
        val = -val;
    }
    return result_ok(val);
}

[[nodiscard]] inline bool is_obsolete_flag(const char* arg) noexcept {
    if (arg == nullptr || arg[0] != '-') {
        return false;
    }
    if (hosted::cstrcmp(arg, "--") == 0) {
        return false;
    }
    if (arg[1] == '-' || (arg[1] >= '0' && arg[1] <= '9')) {
        return true;
    }
    return false;
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
        if (hosted::cstrcmp(arg, "-n") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            auto parsed = parse_nice_value(argv[++i]);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.increment = parsed.value;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i < 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline long combined_priority(long current, long increment) noexcept {
    return current + increment;
}

} // namespace pbsd::userland::usr_bin::nice
