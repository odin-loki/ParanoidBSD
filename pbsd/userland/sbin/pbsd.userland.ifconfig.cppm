module;
#include <cstddef>

export module pbsd.userland.ifconfig;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/sbin/ifconfig/ifconfig.c — flag parse stubs (logic-only).
export namespace pbsd::userland::sbin::ifconfig {

struct Options {
    bool all{false};
    bool downonly{false};
    bool drivername{false};
    bool printkeys{false};
    bool namesonly{false};
    bool media{false};
    bool verbose{false};
    const char* nogroup{nullptr};
    const char* format{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* flag = argv[i];
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'a':
                opt.all = true;
                break;
            case 'd':
                opt.downonly = true;
                break;
            case 'D':
                opt.drivername = true;
                break;
            case 'k':
                opt.printkeys = true;
                break;
            case 'l':
                opt.namesonly = true;
                break;
            case 'm':
                opt.media = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'f':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.format = argv[++i];
                    goto next_flag;
                }
                opt.format = flag + j + 1;
                goto next_flag;
            case 'G':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc || !opt.all) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.nogroup = argv[++i];
                    goto next_flag;
                }
                if (!opt.all) {
                    return result_err<Options>(Status::Invalid);
                }
                opt.nogroup = flag + j + 1;
                goto next_flag;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    next_flag:;
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline const char* peek_arg(int argc, char* const* argv,
                                          int idx) noexcept {
    if (argv == nullptr || idx >= argc) {
        return nullptr;
    }
    return argv[idx];
}

} // namespace pbsd::userland::sbin::ifconfig
