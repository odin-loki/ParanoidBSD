module;
#include <cstddef>

export module pbsd.userland.col;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/col/col.c — filter reverse line feeds.
export namespace pbsd::userland::usr_bin::col {

inline constexpr int kBufferMargin = 32;
inline constexpr int kDefaultMaxBufdLines = 256;

enum class CharSet : unsigned char {
    Normal = 1,
    Alternate = 2,
};

struct Options {
    bool no_backspaces{false};      // -b
    bool fine{false};               // -f half-line feeds
    bool compress_spaces{true};     // -h / default
    int max_bufd_lines{kDefaultMaxBufdLines * 2}; // -l (half-lines)
    bool pass_unknown_seqs{false};  // -p
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
        if (hosted::cstrcmp(arg, "--") == 0) {
            ++i;
            break;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'b':
                opt.no_backspaces = true;
                break;
            case 'f':
                opt.fine = true;
                break;
            case 'h':
                opt.compress_spaces = true;
                break;
            case 'p':
                opt.pass_unknown_seqs = true;
                break;
            case 'x':
                opt.compress_spaces = false;
                break;
            case 'l':
                ++j;
                while (arg[j] != '\0') {
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

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "bfhl:px";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::col
