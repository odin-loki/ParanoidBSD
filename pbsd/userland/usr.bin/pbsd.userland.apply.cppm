module;
#include <cstddef>

export module pbsd.userland.apply;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/apply/apply.c — apply command to arguments.
export namespace pbsd::userland::usr_bin::apply {

inline constexpr char kDefaultMagic = '%';

struct Options {
    char magic{kDefaultMagic}; // -a magic or -0123456789
    int nargs{-1};             // -N digit
    bool debug{false};         // -d
};

[[nodiscard]] inline bool is_magic_number(const char* p, char magic) noexcept {
    if (p == nullptr || p[0] != magic) {
        return false;
    }
    const unsigned char d = static_cast<unsigned char>(p[1]);
    return d >= '1' && d <= '9';
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
        if (arg[1] == 'a' && arg[2] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        if (arg[1] == 'a' && arg[2] != '\0') {
            if (arg[2] == '\0' || arg[3] != '\0') {
                return result_err<Options>(Status::Invalid);
            }
            opt.magic = arg[2];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'd':
                opt.debug = true;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                opt.nargs = arg[j] - '0';
                break;
            case 'a':
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

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "a:d0123456789";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::apply
