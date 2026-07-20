module;
#include <cstddef>

export module pbsd.userland.reboot;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/reboot — flag parse scaffold (logic-only).
export namespace pbsd::userland::sbin::reboot {

struct Options {
    bool force{false};
    bool quiet{false};
    bool verbose{false};
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'f':
                opt.force = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "dnqrv";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}
} // namespace pbsd::userland::sbin::reboot
