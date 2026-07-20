module;
#include <cstddef>

export module pbsd.userland.jot;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/jot/jot.c — flag/operand helpers (logic-only).
export namespace pbsd::userland::usr_bin::jot {

inline constexpr long kRepsDefault = 100;
inline constexpr double kBeginDefault = 1.0;
inline constexpr double kEndDefault = 100.0;
inline constexpr double kStepDefault = 1.0;

struct Options {
    bool boring{false};
    bool char_data{false};
    bool no_final_nl{false};
    bool randomize{false};
    int precision{-1};
    const char* separator{"\n"};
    const char* format{nullptr};
    double begin{kBeginDefault};
    double end{kEndDefault};
    double step{kStepDefault};
    long reps{kRepsDefault};
};

[[nodiscard]] inline bool is_default_arg(const char* s) noexcept {
    return s == nullptr || s[0] == '\0' || hosted::cstrcmp(s, "-") == 0;
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
        if (arg[1] == 'b') {
            opt.boring = true;
            if (arg[2] == 'w' && arg[3] == '\0') {
                if (i + 1 >= argc) {
                    return result_err<Options>(Status::Invalid);
                }
                opt.format = argv[++i];
            }
            continue;
        }
        if (arg[1] == 'w' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.format = argv[++i];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'c':
                opt.char_data = true;
                break;
            case 'n':
                opt.no_final_nl = true;
                break;
            case 'r':
                opt.randomize = true;
                break;
            case 'p':
                return result_err<Options>(Status::Invalid);
            case 's':
                return result_err<Options>(Status::Invalid);
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    const int nargs = argc - i;
    if (nargs < 1 || nargs > 4) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline int getprec(const char* s) noexcept {
    if (s == nullptr) {
        return 0;
    }
    const char* dot = s;
    while (*dot != '\0' && *dot != '.') {
        ++dot;
    }
    if (*dot != '.') {
        return 0;
    }
    int n = 0;
    for (++dot; *dot >= '0' && *dot <= '9'; ++dot) {
        ++n;
    }
    return n;
}

} // namespace pbsd::userland::usr_bin::jot
