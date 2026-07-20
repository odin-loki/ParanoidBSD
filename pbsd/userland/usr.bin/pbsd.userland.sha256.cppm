module;
#include <cstddef>

export module pbsd.userland.sha256;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/lib/libmd/sha256 — checksum flags (logic-only).
export namespace pbsd::userland::usr_bin::sha256 {

inline constexpr int kDigestLength = 32;
inline constexpr int kHexChars = 64;

struct Options {
    bool check{false};
    bool quiet{false};
    bool pseudo{false};
    bool reverse{false};
    bool tag{false};
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
            case 'c':
                opt.check = true;
                break;
            case 'p':
                opt.pseudo = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'r':
                opt.reverse = true;
                break;
            case 't':
                opt.tag = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool is_hex_digit(char c) noexcept {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

[[nodiscard]] inline int hex_value(char c) noexcept {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

[[nodiscard]] inline bool valid_digest_line(const char* line) noexcept {
    if (line == nullptr) {
        return false;
    }
    int hex_count = 0;
    for (const char* p = line; *p != '\0'; ++p) {
        if (*p == ' ' || *p == '\t') {
            if (hex_count == kHexChars) {
                return true;
            }
            return false;
        }
        if (!is_hex_digit(*p)) {
            return false;
        }
        ++hex_count;
        if (hex_count > kHexChars) {
            return false;
        }
    }
    return hex_count == kHexChars;
}

} // namespace pbsd::userland::usr_bin::sha256
