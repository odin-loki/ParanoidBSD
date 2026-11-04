module;
#include <cstddef>

export module pbsd.userland.zic;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.sbin/zic/zic.c — timezone compiler (logic-only).
export namespace pbsd::userland::usr_sbin::zic {

struct Options {
    bool verbose{false};
    bool quiet{false};
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
        if (hosted::cstrcmp(arg, "--") == 0) {
            ++i;
            break;
        }
        if (hosted::cstrcmp(arg, "-v") == 0) {
            opt.verbose = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    return c == 'v' || c == 'q';
}

} // namespace pbsd::userland::usr_sbin::zic
