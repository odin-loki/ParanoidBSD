module;
#include <cstddef>

export module pbsd.userland.uncompress;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/compress/compress.c — decompress entry detection (logic-only).
export namespace pbsd::userland::usr_bin::uncompress {

enum class Style {
    Compress,
    Decompress,
};

inline constexpr unsigned char kCompressMagic0 = 0x1f;
inline constexpr unsigned char kCompressMagic1 = 0x9d;
inline constexpr const char* kSuffix = ".Z";

struct Options {
    Style style{Style::Decompress};
    bool cat_mode{false};
    int bits{0};
    bool force{false};
    bool verbose{false};
};

[[nodiscard]] inline Style style_from_progname(const char* progname) noexcept {
    if (progname == nullptr) {
        return Style::Decompress;
    }
    const char* base = progname;
    for (const char* p = progname; *p != '\0'; ++p) {
        if (*p == '/') {
            base = p + 1;
        }
    }
    if (hosted::cstrcmp(base, "compress") == 0) {
        return Style::Compress;
    }
    if (hosted::cstrcmp(base, "uncompress") == 0) {
        return Style::Decompress;
    }
    if (hosted::cstrcmp(base, "zcat") == 0) {
        return Style::Decompress;
    }
    return Style::Decompress;
}

[[nodiscard]] inline bool is_zcat(const char* progname) noexcept {
    if (progname == nullptr) {
        return false;
    }
    const char* base = progname;
    for (const char* p = progname; *p != '\0'; ++p) {
        if (*p == '/') {
            base = p + 1;
        }
    }
    return hosted::cstrcmp(base, "zcat") == 0;
}

[[nodiscard]] inline bool has_compress_magic(const unsigned char* data, std::size_t len) noexcept {
    return len >= 2 && data[0] == kCompressMagic0 && data[1] == kCompressMagic1;
}

[[nodiscard]] inline bool has_suffix_Z(const char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    std::size_t len = 0;
    while (path[len] != '\0') {
        ++len;
    }
    if (len < 2) {
        return false;
    }
    return path[len - 2] == '.' && path[len - 1] == 'Z';
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out,
                                                const char* progname) noexcept {
    Options opt{};
    opt.style = style_from_progname(progname);
    opt.cat_mode = is_zcat(progname);
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'b':
                if (i + 1 >= argc) {
                    return result_err<Options>(Status::Invalid);
                }
                {
                    const char* bits_arg = argv[++i];
                    int bits = 0;
                    for (const char* p = bits_arg; *p >= '0' && *p <= '9'; ++p) {
                        bits = bits * 10 + (*p - '0');
                    }
                    opt.bits = bits;
                }
                break;
            case 'c':
                opt.cat_mode = true;
                break;
            case 'f':
                opt.force = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::uncompress
