module;
#include <cstddef>

export module pbsd.userland.sort;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sort/sort.c — flag/key helpers (logic-only).
export namespace pbsd::userland::usr_bin::sort {

struct Options {
    bool check{false};
    bool unique{false};
    bool reverse{false};
    bool numeric{false};
    bool human{false};
    bool month{false};
    bool ignore_case{false};
    bool stable{false};
    bool version{false};
    bool randomize{false};
    bool zero_term{false};
    char separator{'\0'};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "--version") == 0) {
            opt.version = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "--zero-terminated") == 0) {
            opt.zero_term = true;
            continue;
        }
        if (arg[1] == 't' && arg[2] != '\0') {
            opt.separator = arg[2];
            continue;
        }
        if (arg[1] == 't' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.separator = argv[++i][0];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'c':
                opt.check = true;
                break;
            case 'u':
                opt.unique = true;
                break;
            case 'r':
                opt.reverse = true;
                break;
            case 'n':
                opt.numeric = true;
                break;
            case 'h':
                opt.human = true;
                break;
            case 'M':
                opt.month = true;
                break;
            case 'f':
                opt.ignore_case = true;
                break;
            case 's':
                opt.stable = true;
                break;
            case 'R':
                opt.randomize = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (opt.check && opt.unique) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool key_field_valid(unsigned field) noexcept {
    return field > 0;
}

} // namespace pbsd::userland::usr_bin::sort
