module;
#include <cstddef>

export module pbsd.userland.join;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/join/join.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::join {

struct Options {
    bool unpairable1{false};
    bool unpairable2{false};
    bool check_order{false};
    bool ignore_case{false};
    char separator{' '};
    char empty{'\0'};
    unsigned field1{0};
    unsigned field2{0};
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
        if (arg[1] == '1' && arg[2] != '\0') {
            opt.field1 = static_cast<unsigned>(arg[2] - '0');
            continue;
        }
        if (arg[1] == '2' && arg[2] != '\0') {
            opt.field2 = static_cast<unsigned>(arg[2] - '0');
            continue;
        }
        if (arg[1] == 'o' && arg[2] != '\0') {
            continue;
        }
        if (arg[1] == 't' && arg[2] != '\0') {
            opt.separator = arg[2];
            continue;
        }
        if (arg[1] == 'e' && arg[2] != '\0') {
            opt.empty = arg[2];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'a':
                return result_err<Options>(Status::Invalid);
            case 'v':
                return result_err<Options>(Status::Invalid);
            case 'i':
                opt.ignore_case = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (argc - i < 2) {
        return result_err<Options>(Status::Invalid);
    }
    if (opt.field1 == 0) {
        opt.field1 = 1;
    }
    if (opt.field2 == 0) {
        opt.field2 = 1;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::join
