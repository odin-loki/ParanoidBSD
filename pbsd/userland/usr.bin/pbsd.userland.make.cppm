module;
#include <cstddef>

export module pbsd.userland.make;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/contrib/bmake/main.c — BSD make (bmake).
export namespace pbsd::userland::usr_bin::make {

inline constexpr const char kOptSpecs[] = "BC:D:I:J:NST:V:WXd:ef:ij:km:nqrstv:w";

struct Options {
    bool enter_flag{false};   // -C
    bool ignore_errors{false}; // -i
    bool keep_going{false};   // -k
    bool no_exec{false};      // -n
    bool query{false};        // -q
    bool touch{false};        // -t
    bool dry_run{false};
    bool warn_order{false};   // -W
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    for (const char* p = kOptSpecs; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool takes_argument(char c) noexcept {
    const char* p = hosted::cstrchr(kOptSpecs, c);
    return p != nullptr && p[1] == ':';
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
        if (arg[1] == '-' && arg[2] == '\0') {
            ++i;
            break;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'i':
                opt.ignore_errors = true;
                break;
            case 'k':
                opt.keep_going = true;
                break;
            case 'n':
                opt.no_exec = true;
                opt.dry_run = true;
                break;
            case 'q':
                opt.query = true;
                break;
            case 't':
                opt.touch = true;
                break;
            case 'W':
                opt.warn_order = true;
                break;
            case 'B':
            case 'C':
            case 'D':
            case 'I':
            case 'J':
            case 'N':
            case 'S':
            case 'T':
            case 'V':
            case 'X':
            case 'd':
            case 'e':
            case 'f':
            case 'j':
            case 'm':
            case 'r':
            case 's':
            case 'v':
            case 'w':
                if (takes_argument(arg[j])) {
                    while (arg[j + 1] != '\0') {
                        ++j;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::make
