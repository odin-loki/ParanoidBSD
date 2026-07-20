module;
#include <cstddef>

export module pbsd.userland.egrep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/grep — ERE (egrep) mode detection (logic-only).
export namespace pbsd::userland::usr_bin::egrep {

struct Options {
    bool extended_regex{true};
    bool ignore_case{false};
    bool line_regexp{false};
    bool word_regexp{false};
    bool invert_match{false};
    bool count_only{false};
    bool files_without_match{false};
    bool files_with_match{false};
    bool no_filename{false};
    bool line_number{false};
    bool quiet{false};
};

[[nodiscard]] inline bool is_egrep_progname(const char* progname) noexcept {
    if (progname == nullptr) {
        return false;
    }
    const char* base = progname;
    for (const char* p = progname; *p != '\0'; ++p) {
        if (*p == '/') {
            base = p + 1;
        }
    }
    return hosted::cstrcmp(base, "egrep") == 0 || hosted::cstrcmp(base, "Egrep") == 0;
}

[[nodiscard]] inline Result<Options> default_egrep_options() noexcept {
    Options opt{};
    opt.extended_regex = true;
    return result_ok(opt);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                  int& optind_out,
                                                  const char* progname) noexcept {
    Options opt{};
    opt.extended_regex = is_egrep_progname(progname);
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
            case 'E':
                opt.extended_regex = true;
                break;
            case 'F':
                opt.extended_regex = false;
                break;
            case 'i':
                opt.ignore_case = true;
                break;
            case 'v':
                opt.invert_match = true;
                break;
            case 'c':
                opt.count_only = true;
                break;
            case 'l':
                opt.files_with_match = true;
                break;
            case 'L':
                opt.files_without_match = true;
                break;
            case 'h':
                opt.no_filename = true;
                break;
            case 'n':
                opt.line_number = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'w':
                opt.word_regexp = true;
                break;
            case 'x':
                opt.line_regexp = true;
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool needs_pattern(int argc, int optind) noexcept {
    return argc - optind >= 1;
}

} // namespace pbsd::userland::usr_bin::egrep
