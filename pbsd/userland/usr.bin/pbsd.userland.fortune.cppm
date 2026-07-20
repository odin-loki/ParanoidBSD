module;
#include <cstddef>

export module pbsd.userland.fortune;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/games/fortune/fortune/fortune.c — fortune selection (logic-only).
export namespace pbsd::userland::usr_bin::fortune {

struct Options {
    bool show_all{false};
    bool long_only{false};
    bool offensive{false};
    int repeat_count{1};
};

[[nodiscard]] inline Result<std::size_t> pick_index(std::size_t count,
                                                      unsigned seed) noexcept {
    if (count == 0) {
        return result_ok(static_cast<std::size_t>(0));
    }
    return result_ok(seed % count);
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'a':
                opt.show_all = true;
                break;
            case 'l':
                opt.long_only = true;
                break;
            case 'o':
                opt.offensive = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (argc - i == 1) {
        int val = 0;
        for (const char* p = argv[i]; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<Options>(Status::Invalid);
            }
            val = val * 10 + (*p - '0');
        }
        if (val < 1) {
            return result_err<Options>(Status::Invalid);
        }
        opt.repeat_count = val;
    } else if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::fortune
