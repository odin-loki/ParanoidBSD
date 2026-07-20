module;
#include <cstddef>

export module pbsd.userland.yacc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/contrib/byacc/main.c — LALR parser generator.
export namespace pbsd::userland::usr_bin::yacc {

struct Options {
    bool debug{false};        // -d
    bool graph{false};        // -g
    bool verbose{false};      // -v
    bool line_directives{false}; // -l
    bool token_table{false};  // -t
    bool backtrack{false};    // -B
    bool pure_parser{false};  // -P
    bool locations{false};    // -L
    bool posix_yacc{true};    // -y (noop, default)
    const char* file_prefix{nullptr}; // -b
    const char* output_file{nullptr}; // -o
    const char* symbol_prefix{nullptr}; // -p
    const char* defines_file{nullptr}; // -H
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "Bb:dghH:ilLo:Pp:rstVvy";
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
            case 'd':
                opt.debug = true;
                break;
            case 'g':
                opt.graph = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'l':
                opt.line_directives = true;
                break;
            case 't':
                opt.token_table = true;
                break;
            case 'B':
                opt.backtrack = true;
                break;
            case 'P':
                opt.pure_parser = true;
                break;
            case 'L':
                opt.locations = true;
                break;
            case 'y':
                opt.posix_yacc = true;
                break;
            case 'b':
            case 'H':
            case 'o':
            case 'p':
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 'h':
                return result_err<Options>(Status::Invalid);
            case 'V':
                return result_ok(opt);
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::yacc
