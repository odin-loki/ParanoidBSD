module;
#include <cstddef>

export module pbsd.userland.su;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/su/su.c — substitute user identity (PAM).
export namespace pbsd::userland::usr_bin::su {

enum class LoginStyle : unsigned char {
    Unset,
    Full,    // - / -l (login shell as target)
    Minimal, // -m (preserve environment)
};

struct Options {
    bool fast_login{false}; // -f
    bool set_maclabel{false}; // -s
    LoginStyle style{LoginStyle::Unset};
    const char* login_class{nullptr}; // -c
    const char* user{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "-flmsc:";
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
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-' || hosted::cstrcmp(arg, "--") == 0) {
            break;
        }
        if (hosted::cstrcmp(arg, "-") == 0) {
            opt.style = LoginStyle::Full;
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'f':
                opt.fast_login = true;
                break;
            case 'l':
                opt.style = LoginStyle::Full;
                break;
            case 'm':
                opt.style = LoginStyle::Minimal;
                break;
            case 's':
                opt.set_maclabel = true;
                break;
            case 'c':
                while (arg[j + 1] != '\0') {
                    ++j;
                }
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

} // namespace pbsd::userland::usr_bin::su
