module;
#include <cstddef>

export module pbsd.userland.xargs;

export import pbsd.core;

/// Port helpers from hbsd/src/usr.bin/xargs/xargs.c — option parse + argv packing.
export namespace pbsd::userland::usr_bin::xargs {

struct Options {
    std::size_t max_args{0};   // -n
    std::size_t max_chars{0};  // -s
    bool exit_on_empty{false}; // -r (GNU; FreeBSD often default)
    bool verbose{false};       // -t
    char delim{'\0'};          // -0 → NUL, else whitespace
    bool null_delim{false};
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case '0':
                opt.null_delim = true;
                opt.delim = '\0';
                break;
            case 't':
                opt.verbose = true;
                break;
            case 'r':
                opt.exit_on_empty = true;
                break;
            case 'n':
            case 's': {
                bool nargs = (*p == 'n');
                const char* arg = (p[1] != '\0') ? p + 1 : (i + 1 < argc ? argv[++i] : nullptr);
                if (!arg) {
                    return result_err<Options>(Status::Invalid);
                }
                std::size_t n = 0;
                for (const char* q = arg; *q; ++q) {
                    if (*q < '0' || *q > '9') {
                        return result_err<Options>(Status::Invalid);
                    }
                    n = n * 10 + static_cast<std::size_t>(*q - '0');
                }
                if (nargs) {
                    opt.max_args = n;
                } else {
                    opt.max_chars = n;
                }
                if (p[1] != '\0') {
                    while (p[1]) {
                        ++p;
                    }
                }
                break;
            }
            case 'E':
            case 'I':
            case 'J':
            case 'L':
            case 'P':
                if (p[1] == '\0') {
                    ++i;
                }
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Would adding `token` exceed -n/-s limits given current packed size?
[[nodiscard]] inline bool fits(const Options& o, std::size_t cur_args, std::size_t cur_chars,
                               std::size_t token_len) noexcept {
    if (o.max_args != 0 && cur_args + 1 > o.max_args) {
        return false;
    }
    if (o.max_chars != 0 && cur_chars + token_len + 1 > o.max_chars) {
        return false;
    }
    return true;
}

} // namespace pbsd::userland::usr_bin::xargs
