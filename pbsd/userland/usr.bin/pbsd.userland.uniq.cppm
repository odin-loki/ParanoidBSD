module;
#include <cstddef>

export module pbsd.userland.uniq;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/uniq/uniq.c.
export namespace pbsd::userland::usr_bin::uniq {

struct Options {
    bool count{false};   // -c
    bool repeated{false};// -d
    bool unique{false};  // -u
    std::size_t skip_fields{0}; // -f
    std::size_t skip_chars{0};  // -s
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
            case 'c':
                opt.count = true;
                break;
            case 'd':
                opt.repeated = true;
                break;
            case 'u':
                opt.unique = true;
                break;
            case 'i':
                break;
            case 'f':
            case 's': {
                bool fields = (*p == 'f');
                const char* arg = (p[1] != '\0') ? p + 1 : (i + 1 < argc ? argv[++i] : nullptr);
                if (arg == nullptr) {
                    return result_err<Options>(Status::Invalid);
                }
                std::size_t n = 0;
                for (const char* q = arg; *q; ++q) {
                    if (*q < '0' || *q > '9') {
                        return result_err<Options>(Status::Invalid);
                    }
                    n = n * 10 + static_cast<std::size_t>(*q - '0');
                }
                if (fields) {
                    opt.skip_fields = n;
                } else {
                    opt.skip_chars = n;
                }
                if (p[1] != '\0') {
                    while (p[1]) {
                        ++p;
                    }
                }
                break;
            }
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool lines_equal(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b) == 0;
}

} // namespace pbsd::userland::usr_bin::uniq
