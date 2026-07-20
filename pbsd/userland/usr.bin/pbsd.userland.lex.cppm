module;
#include <cstddef>

export module pbsd.userland.lex;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/lex/initparse.h — lexical analyzer generator tokens.
export namespace pbsd::userland::usr_bin::lex {

inline constexpr int kTokChar = 257;
inline constexpr int kTokNumber = 258;
inline constexpr int kTokSectEnd = 259;
inline constexpr int kTokScDecl = 260;
inline constexpr int kTokXScDecl = 261;
inline constexpr int kTokName = 262;
inline constexpr int kTokPrevCcl = 263;
inline constexpr int kTokEofOp = 264;
inline constexpr int kTokOption = 265;
inline constexpr int kTokOutfile = 266;
inline constexpr int kTokPrefix = 267;
inline constexpr int kTokYyclass = 268;
inline constexpr int kTokHeaderFile = 269;
inline constexpr int kTokExtraType = 270;
inline constexpr int kTokTablesFile = 271;

struct Options {
    bool full_table{false};   // -f
    bool case_insensitive{false}; // -i
    bool verbose{false};      // -v
    bool eight_bit{true};     // -8 default
    const char* outfile{nullptr};
    const char* prefix{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "8bcfIlnpsStv";
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
            case '8':
                opt.eight_bit = true;
                break;
            case 'f':
                opt.full_table = true;
                break;
            case 'i':
                opt.case_insensitive = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'o':
            case 'P':
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

} // namespace pbsd::userland::usr_bin::lex
