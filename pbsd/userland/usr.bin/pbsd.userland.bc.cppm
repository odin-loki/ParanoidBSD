module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.bc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/contrib/bc/src/{bc.c,args.c} — arbitrary precision calculator.
export namespace pbsd::userland::usr_bin::bc {

enum Flag : std::uint32_t {
    Interactive = 0x01U,
    GlobalLib = 0x02U,
    Quiet = 0x04U,
    NoReadPrompt = 0x08U,
    Seed = 0x10U,
};

struct Options {
    std::uint32_t flags{Flag::Quiet};
    bool no_exprs{false};
    bool exit_exprs{false};
    int line_len{-1};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "cefg:h:i:I:l:O:Pq:r:s:S:z:E";
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
            case 'i':
                opt.flags |= Flag::Interactive;
                break;
            case 'g':
                opt.flags |= Flag::GlobalLib;
                break;
            case 'l':
                opt.flags |= Flag::GlobalLib;
                break;
            case 'q':
                opt.flags &= ~Flag::Quiet;
                break;
            case 'L':
                opt.line_len = 0;
                break;
            case 'P':
            case 'R':
            case 'z':
                break;
            case 'f':
            case 'I':
            case 'O':
            case 'S':
            case 'E':
            case 'r':
            case 's':
            case 'c':
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
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::bc
