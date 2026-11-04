module;
#include <cstddef>

export module pbsd.userland.grep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/grep — flag parse + literal match (ERE later).
export namespace pbsd::userland::usr_bin::grep {

struct Options {
    bool ignore_case{false}; // -i
    bool invert{false};      // -v
    bool count_only{false};  // -c
    bool list_files{false};  // -l
    bool quiet{false};       // -q
    bool word{false};        // -w
    bool line_regexp{false}; // -x
    bool fixed{false};       // -F
    bool line_number{false}; // -n
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        if (argv[i][1] == 'e' || argv[i][1] == 'f') {
            if (argv[i][2] == '\0') {
                ++i; // pattern/file arg
            }
            continue;
        }
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'i':
                opt.ignore_case = true;
                break;
            case 'v':
                opt.invert = true;
                break;
            case 'c':
                opt.count_only = true;
                break;
            case 'l':
                opt.list_files = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'w':
                opt.word = true;
                break;
            case 'x':
                opt.line_regexp = true;
                break;
            case 'F':
                opt.fixed = true;
                break;
            case 'n':
                opt.line_number = true;
                break;
            case 'E':
            case 'G':
            case 'r':
            case 'R':
            case 'H':
            case 'h':
            case 's':
            case 'o':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline char fold(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

/// Literal substring search; returns true if pattern found in line.
[[nodiscard]] inline bool match_fixed(const char* line, const char* pat, bool icase) noexcept {
    if (line == nullptr || pat == nullptr || pat[0] == '\0') {
        return false;
    }
    for (std::size_t i = 0; line[i]; ++i) {
        std::size_t j = 0;
        for (; pat[j]; ++j) {
            char a = line[i + j];
            char b = pat[j];
            if (a == '\0') {
                return false;
            }
            if (icase) {
                a = fold(a);
                b = fold(b);
            }
            if (a != b) {
                break;
            }
        }
        if (pat[j] == '\0') {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::grep
