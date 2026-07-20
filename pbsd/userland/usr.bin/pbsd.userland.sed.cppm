module;
#include <cstddef>

export module pbsd.userland.sed;

export import pbsd.core;

/// Port helpers from hbsd/src/usr.bin/sed — flag parse + s/// command scaffold.
export namespace pbsd::userland::usr_bin::sed {

struct Options {
    bool extended{false};  // -E / -r
    bool quiet{false};     // -n
    bool unbuffered{false};// -u
    bool inplace{false};   // -i
};

enum class CmdKind : unsigned char { Substitute, Delete, Print, Unknown };

struct SubCmd {
    char delim{'/'};
    const char* pattern{nullptr};
    const char* repl{nullptr};
    bool global{false};
    bool print{false};
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        if (argv[i][1] == 'e' || argv[i][1] == 'f' || argv[i][1] == 'i') {
            if (argv[i][2] == '\0' && argv[i][1] != 'i') {
                ++i;
            }
            if (argv[i][1] == 'i') {
                opt.inplace = true;
            }
            continue;
        }
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'E':
            case 'r':
                opt.extended = true;
                break;
            case 'n':
                opt.quiet = true;
                break;
            case 'u':
                opt.unbuffered = true;
                break;
            case 'a':
            case 'I':
            case 'l':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Parse leading `s/pat/repl/flags` — pointers into `expr` (not copied).
[[nodiscard]] inline Result<SubCmd> parse_substitute(const char* expr) noexcept {
    SubCmd c{};
    if (expr == nullptr || expr[0] != 's' || expr[1] == '\0') {
        return result_err<SubCmd>(Status::Invalid);
    }
    c.delim = expr[1];
    const char* p = expr + 2;
    c.pattern = p;
    while (*p && *p != c.delim) {
        ++p;
    }
    if (*p != c.delim) {
        return result_err<SubCmd>(Status::Invalid);
    }
    // pattern ends at p; we don't mutate — caller uses lengths via next delim
    ++p;
    c.repl = p;
    while (*p && *p != c.delim) {
        ++p;
    }
    if (*p != c.delim) {
        return result_err<SubCmd>(Status::Invalid);
    }
    ++p;
    for (; *p; ++p) {
        if (*p == 'g') {
            c.global = true;
        } else if (*p == 'p') {
            c.print = true;
        }
    }
    return result_ok(c);
}

[[nodiscard]] inline CmdKind classify(const char* expr) noexcept {
    if (expr == nullptr || expr[0] == '\0') {
        return CmdKind::Unknown;
    }
    if (expr[0] == 's') {
        return CmdKind::Substitute;
    }
    if (expr[0] == 'd') {
        return CmdKind::Delete;
    }
    if (expr[0] == 'p') {
        return CmdKind::Print;
    }
    return CmdKind::Unknown;
}

} // namespace pbsd::userland::usr_bin::sed
