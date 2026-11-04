module;
#include <cstddef>

export module pbsd.userland.soelim;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/soelim/soelim.c — include preprocessing (logic-only).
export namespace pbsd::userland::usr_bin::soelim {

inline constexpr unsigned kCOption = 0x1;

struct Options {
    unsigned flags{0};
    int include_count{0};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'C':
                opt.flags |= kCOption;
                break;
            case 'I':
                ++opt.include_count;
                if (arg[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    ++i;
                }
                break;
            case 'r':
            case 't':
            case 'v':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool is_stdin_name(const char* name) noexcept {
    return name != nullptr && hosted::cstrcmp(name, "-") == 0;
}

[[nodiscard]] inline bool is_so_directive(const char* line) noexcept {
    if (line == nullptr) {
        return false;
    }
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    return line[0] == '.' && line[1] == 's' && line[2] == 'o' &&
           (line[3] == ' ' || line[3] == '\t');
}

} // namespace pbsd::userland::usr_bin::soelim
