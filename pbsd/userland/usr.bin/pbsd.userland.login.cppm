module;
#include <cstddef>

export module pbsd.userland.login;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/login/{login.c,pathnames.h}.
export namespace pbsd::userland::usr_bin::login {

inline constexpr const char kHushLogin[] = ".hushlogin";
inline constexpr const char kMotdFile[] = "/var/run/motd";
inline constexpr const char kFbtab[] = "/etc/fbtab";
inline constexpr const char kLoginDevPerm[] = "/etc/logindevperm";

struct Options {
    bool preauth{false};   // -f (pre-authenticated)
    bool preserve_env{false}; // -p
    const char* hostname{nullptr}; // -h
    const char* user{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "fh:p";
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
            case 'f':
                opt.preauth = true;
                break;
            case 'p':
                opt.preserve_env = true;
                break;
            case 'h':
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

} // namespace pbsd::userland::usr_bin::login
