module;
#include <cstddef>

export module pbsd.userland.diff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/diff.c — flag/algorithm helpers (logic-only).
export namespace pbsd::userland::usr_bin::diff {

enum class Algorithm : unsigned char { None, Stone, Myers, Patience };

struct Options {
    bool brief{false};
    bool context{false};
    bool unified{false};
    bool side_by_side{false};
    bool ignore_case{false};
    bool recursive{false};
    bool suppress_common{false};
    bool color{false};
    int context_lines{3};
    Algorithm algorithm{Algorithm::Myers};
};

[[nodiscard]] inline Result<Algorithm> algorithm_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<Algorithm>(Status::Invalid);
    }
    if (hosted::cstrcmp(name, "stone") == 0) {
        return result_ok(Algorithm::Stone);
    }
    if (hosted::cstrcmp(name, "myers") == 0) {
        return result_ok(Algorithm::Myers);
    }
    if (hosted::cstrcmp(name, "patience") == 0) {
        return result_ok(Algorithm::Patience);
    }
    return result_err<Algorithm>(Status::Invalid);
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
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "--help") == 0) {
            continue;
        }
        if (hosted::cstrcmp(arg, "--suppress-common-lines") == 0) {
            opt.suppress_common = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "--color") == 0) {
            opt.color = true;
            continue;
        }
        if (arg[1] == 'U' && arg[2] != '\0') {
            opt.unified = true;
            opt.context_lines = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.context_lines = opt.context_lines * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'C' && arg[2] != '\0') {
            opt.context = true;
            opt.context_lines = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.context_lines = opt.context_lines * 10 + (*p - '0');
            }
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'b':
                opt.brief = true;
                break;
            case 'c':
                opt.context = true;
                break;
            case 'u':
                opt.unified = true;
                break;
            case 'y':
                opt.side_by_side = true;
                break;
            case 'i':
                opt.ignore_case = true;
                break;
            case 'r':
                opt.recursive = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::diff
