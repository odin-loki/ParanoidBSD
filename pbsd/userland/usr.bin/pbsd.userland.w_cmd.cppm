module;
#include <cstddef>

export module pbsd.userland.w_cmd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/w/w.c — Who-style snapshot flags
export namespace pbsd::userland::usr_bin::w {

struct Options {
    bool short_form{false};
    bool no_header{false};
    bool from_host{false};
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
            case 'h':
                opt.short_form = true;
                break;
            case 'n':
                opt.no_header = true;
                break;
            case 'f':
                opt.from_host = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::w
