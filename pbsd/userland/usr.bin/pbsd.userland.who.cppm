module;
#include <cstddef>

export module pbsd.userland.who;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/who/who.c — -H/-q/-T flags
export namespace pbsd::userland::usr_bin::who {

struct Options {
    bool headings{false};
    bool quick{false};
    bool status{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'H':
                opt.headings = true;
                break;
            case 'q':
                opt.quick = true;
                break;
            case 'T':
                opt.status = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::who
