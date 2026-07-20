module;
#include <cstddef>

export module pbsd.userland.sdiff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sdiff/sdiff.c — side-by-side diff options (logic-only).
export namespace pbsd::userland::usr_bin::sdiff {

inline constexpr int kDefaultWidth = 126;
inline constexpr int kWidthMin = 5;
inline constexpr int kMaxCheck = 768;
inline constexpr int kDefaultTabSize = 8;

struct Options {
    bool suppress_common{false};
    bool left_only_identical{false};
    bool skip_identical{false};
    bool expand_tabs{false};
    int tabsize{kDefaultTabSize};
    int width{kDefaultWidth};
};

[[nodiscard]] inline Result<int> parse_width(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    if (val < kWidthMin) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(val);
}

[[nodiscard]] inline std::size_t column_width(int total_width) noexcept {
    if (total_width < kWidthMin) {
        return 1;
    }
    return static_cast<std::size_t>((total_width - 3) / 2);
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
        if (hosted::cstrcmp(arg, "-s") == 0 || hosted::cstrcmp(arg, "--suppress-common-lines") == 0) {
            opt.suppress_common = true;
            opt.skip_identical = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-l") == 0) {
            opt.left_only_identical = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-t") == 0) {
            opt.expand_tabs = true;
            continue;
        }
        if (arg[1] == 'w' && arg[2] != '\0') {
            auto parsed = parse_width(arg + 2);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.width = parsed.value;
            continue;
        }
        if (hosted::cstrcmp(arg, "-w") == 0 || hosted::cstrcmp(arg, "--width") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            auto parsed = parse_width(argv[++i]);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.width = parsed.value;
            continue;
        }
        if (arg[1] == 'W' && arg[2] != '\0') {
            auto parsed = parse_width(arg + 2);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.tabsize = parsed.value;
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

} // namespace pbsd::userland::usr_bin::sdiff
