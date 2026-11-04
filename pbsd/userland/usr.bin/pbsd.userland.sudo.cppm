module;
#include <cstddef>

export module pbsd.userland.sudo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from typical BSD sudo(8) — flag parse scaffold (HBSD tree has no sudo source).
export namespace pbsd::userland::usr_bin::sudo {

struct Options {
    bool authenticate{false}; // -A
    bool background{false};   // -b
    bool close_stdin{false};  // -S (read password from stdin)
    bool edit{false};         // -e
    bool preserve_env{false}; // -E
    bool login_shell{false};  // -i
    bool list{false};         // -l
    bool non_interactive{false}; // -n
    bool preserve_groups{false}; // -P
    bool shell{false};        // -s
    bool version{false};      // -V
    bool validate{false};     // -v
    const char* user{nullptr}; // -u
    const char* group{nullptr}; // -g
    const char* command{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "Ab:C:EeghHikKlnPpr:s:St:u:Uv";
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
            case 'A':
                opt.authenticate = true;
                break;
            case 'b':
                opt.background = true;
                break;
            case 'E':
                opt.preserve_env = true;
                break;
            case 'e':
                opt.edit = true;
                break;
            case 'i':
                opt.login_shell = true;
                break;
            case 'l':
                opt.list = true;
                break;
            case 'n':
                opt.non_interactive = true;
                break;
            case 'P':
                opt.preserve_groups = true;
                break;
            case 's':
                opt.shell = true;
                break;
            case 'S':
                opt.close_stdin = true;
                break;
            case 'V':
                opt.version = true;
                break;
            case 'v':
                opt.validate = true;
                break;
            case 'u':
            case 'g':
            case 'C':
            case 'H':
            case 'h':
            case 'k':
            case 'K':
            case 'p':
            case 'r':
            case 't':
            case 'U':
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
        opt.command = argv[i];
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::sudo
