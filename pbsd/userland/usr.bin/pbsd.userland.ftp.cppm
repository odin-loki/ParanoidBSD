module;
#include <cstddef>

export module pbsd.userland.ftp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/ftp/ftp.c — Passive/active + login prompt flags
export namespace pbsd::userland::usr_bin::ftp {

struct Options {
    bool passive{true};
    bool verbose{false};
    bool auto_login{true};
    const char* host{nullptr};
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
            opt.host = arg;
            ++i;
            break;
        }
        if (hosted::cstrcmp(arg, "-A") == 0) {
            opt.passive = false;
            continue;
        }
        if (hosted::cstrcmp(arg, "-v") == 0) {
            opt.verbose = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-n") == 0) {
            opt.auto_login = false;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::ftp
