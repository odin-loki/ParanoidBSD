module;
#include <cstddef>

export module pbsd.userland.base64;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/base64/base64.c — RFC 4648 helpers (logic-only).
export namespace pbsd::userland::usr_bin::base64 {

inline constexpr char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct Options {
    bool decode{false};
    bool ignore_garbage{false};
    int wrap_columns{0};
};

[[nodiscard]] inline int decode_value(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    if (c == '=') {
        return -2;
    }
    return -1;
}

[[nodiscard]] inline bool is_base64_char(char c) noexcept {
    return decode_value(c) >= -2;
}

[[nodiscard]] inline Result<std::size_t> encode_block(const unsigned char* in, std::size_t inlen,
                                                      char* out, std::size_t outlen) noexcept {
    if (in == nullptr || out == nullptr) {
        return result_err<std::size_t>(Status::Invalid);
    }
    std::size_t o = 0;
    for (std::size_t i = 0; i + 2 < inlen; i += 3) {
        if (o + 4 >= outlen) {
            return result_err<std::size_t>(Status::NoMemory);
        }
        out[o++] = kAlphabet[(in[i] >> 2) & 0x3F];
        out[o++] = kAlphabet[((in[i] & 0x3) << 4) | ((in[i + 1] >> 4) & 0xF)];
        out[o++] = kAlphabet[((in[i + 1] & 0xF) << 2) | ((in[i + 2] >> 6) & 0x3)];
        out[o++] = kAlphabet[in[i + 2] & 0x3F];
    }
    const std::size_t rem = inlen % 3;
    if (rem == 0) {
        if (o < outlen) {
            out[o] = '\0';
        }
        return result_ok(o);
    }
    if (o + 4 >= outlen) {
        return result_err<std::size_t>(Status::NoMemory);
    }
    const unsigned char a = in[inlen - rem];
    const unsigned char b = rem == 2 ? in[inlen - 1] : 0;
    out[o++] = kAlphabet[(a >> 2) & 0x3F];
    out[o++] = kAlphabet[((a & 0x3) << 4) | ((b >> 4) & 0xF)];
    out[o++] = rem == 2 ? kAlphabet[((b & 0xF) << 2)] : '=';
    out[o++] = '=';
    if (o < outlen) {
        out[o] = '\0';
    }
    return result_ok(o);
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
        if (hosted::cstrcmp(arg, "-d") == 0) {
            opt.decode = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-i") == 0) {
            opt.ignore_garbage = true;
            continue;
        }
        if (arg[1] == 'w' && arg[2] != '\0') {
            int cols = 0;
            for (const char* p = arg + 2; *p != '\0'; ++p) {
                if (*p < '0' || *p > '9') {
                    return result_err<Options>(Status::Invalid);
                }
                cols = cols * 10 + (*p - '0');
            }
            opt.wrap_columns = cols;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::base64
