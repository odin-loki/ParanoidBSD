module;
#include <cstddef>

export module pbsd.userland.passwd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/passwd/passwd.c — PAM password change.
export namespace pbsd::userland::usr_bin::passwd {

struct Options {
    bool local_only{false}; // -l
    bool yp{false};         // -y
    const char* yp_domain{nullptr}; // -d
    const char* yp_host{nullptr};   // -h
    const char* user{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "d:h:loy";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
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
        if (hosted::cstrcmp(arg, "--") == 0) {
            ++i;
            break;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'l':
                opt.local_only = true;
                break;
            case 'y':
                opt.yp = true;
                break;
            case 'd':
            case 'h':
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 'o':
                break;
            default:
                break;
            }
        }
    }
    if (i < argc && argv[i] != nullptr) {
        opt.user = argv[i++];
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::passwd
