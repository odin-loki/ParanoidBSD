module;
#include <cstddef>

export module pbsd.userland.libc.gen.getbsize;

export import pbsd.core;

/// getbsize from hbsd/src/lib/libc/gen/getbsize.c
export namespace pbsd::userland::libc {

inline constexpr long kKb = 1024L;
inline constexpr long kMb = 1024L * 1024L;
inline constexpr long kGb = 1024L * 1024L * 1024L;
inline constexpr long kMaxBlock = kGb;

struct BlockSize {
    long number{512};
    long bytes{512};
    char suffix{'\0'};
};

[[nodiscard]] inline Result<BlockSize> parse_blocksize(const char* env) noexcept {
    BlockSize out{};
    if (env == nullptr || *env == '\0') {
        out.number = 512;
        out.bytes = 512;
        return result_ok(out);
    }
    long n = 0;
    const char* ep = env;
    while (*ep >= '0' && *ep <= '9') {
        n = n * 10 + (*ep - '0');
        ++ep;
    }
    if (n < 0) {
        return result_err<BlockSize>(Status::Invalid);
    }
    if (n == 0) {
        n = 1;
    }
    long mul = 1;
    char form = '\0';
    if (*ep != '\0' && ep[1] == '\0') {
        switch (*ep) {
        case 'G':
        case 'g':
            form = 'G';
            mul = kGb;
            break;
        case 'M':
        case 'm':
            form = 'M';
            mul = kMb;
            break;
        case 'K':
        case 'k':
            form = 'K';
            mul = kKb;
            break;
        default:
            return result_err<BlockSize>(Status::Invalid);
        }
    } else if (*ep != '\0') {
        return result_err<BlockSize>(Status::Invalid);
    }
    const long max = kMaxBlock / mul;
    if (n > max) {
        n = max;
    }
    long blocksize = n * mul;
    if (blocksize < 512) {
        blocksize = n = 512;
        form = '\0';
    }
    out.number = n;
    out.bytes = blocksize;
    out.suffix = form;
    return result_ok(out);
}

} // namespace pbsd::userland::libc
