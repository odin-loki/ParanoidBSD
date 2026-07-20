module;
#include <cstddef>

export module pbsd.userland.comm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/comm/comm.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::comm {

struct Options {
    bool col1{true};
    bool col2{true};
    bool col3{true};
    bool ignore_case{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p != '\0'; ++p) {
            switch (*p) {
            case '1':
                opt.col1 = false;
                break;
            case '2':
                opt.col2 = false;
                break;
            case '3':
                opt.col3 = false;
                break;
            case 'i':
                opt.ignore_case = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (argc - i != 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline int tab_offset(bool col1, bool col2, bool col3) noexcept {
    return (col1 ? 1 : 0) + (col2 ? 1 : 0) + (col3 ? 1 : 0);
}

} // namespace pbsd::userland::usr_bin::comm
