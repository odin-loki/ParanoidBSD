module;
#include <cstddef>

export module pbsd.userland.fold;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fold/fold.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::fold {

inline constexpr int kDefLineWidth = 80;

struct Options {
    int width{kDefLineWidth};
    bool bytes{false};
    bool split_words{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-b") == 0) {
            opt.bytes = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.split_words = true;
            continue;
        }
        if (arg[1] == 'w' && arg[2] != '\0') {
            opt.width = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.width = opt.width * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'w' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.width = 0;
            for (const char* p = argv[++i]; *p >= '0' && *p <= '9'; ++p) {
                opt.width = opt.width * 10 + (*p - '0');
            }
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (opt.width <= 0) {
        opt.width = kDefLineWidth;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::fold
