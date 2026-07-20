module;
#include <cstddef>

export module pbsd.userland.csplit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/csplit/csplit.c — split options (logic-only).
export namespace pbsd::userland::usr_bin::csplit {

inline constexpr int kDefSuffixLen = 2;
inline constexpr const char* kDefPrefix = "xx";

struct Options {
    const char* prefix{kDefPrefix};
    long suffix_len{kDefSuffixLen};
    bool keep_on_error{false};
    bool suppress_names{false};
};

[[nodiscard]] inline Result<long> parse_suffix_len(const char* s) noexcept {
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
    if (val <= 0) {
        return result_err<long>(Status::Invalid);
    }
    return result_ok(val);
}

[[nodiscard]] inline bool name_too_long(const char* prefix, long suffix_len,
                                        std::size_t path_max) noexcept {
    if (prefix == nullptr) {
        return true;
    }
    const std::size_t plen = hosted::cstrlen(prefix);
    return plen + static_cast<std::size_t>(suffix_len) >= path_max;
}

[[nodiscard]] inline Result<long> max_files_for_suffix(long suffix_len) noexcept {
    if (suffix_len <= 0) {
        return result_err<long>(Status::Invalid);
    }
    long maxfiles = 1;
    for (long i = 0; i < suffix_len; ++i) {
        if (maxfiles > 922337203685477580L) {
            return result_err<long>(Status::Invalid);
        }
        maxfiles *= 10;
    }
    return result_ok(maxfiles);
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
        if (hosted::cstrcmp(arg, "-k") == 0) {
            opt.keep_on_error = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.suppress_names = true;
            continue;
        }
        if (arg[1] == 'f' && arg[2] != '\0') {
            opt.prefix = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.prefix = argv[++i];
            continue;
        }
        if (arg[1] == 'n' && arg[2] != '\0') {
            auto parsed = parse_suffix_len(arg + 2);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.suffix_len = parsed.value;
            continue;
        }
        if (hosted::cstrcmp(arg, "-n") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            auto parsed = parse_suffix_len(argv[++i]);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.suffix_len = parsed.value;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i < 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool is_line_number_expr(const char* expr) noexcept {
    if (expr == nullptr || *expr == '\0') {
        return false;
    }
    for (const char* p = expr; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::userland::usr_bin::csplit
