module;

#include <cstddef>

export module pbsd.userland.libc.gen.unvis;

export import pbsd.core;

/// unvis from hbsd/src/lib/libc/gen/unvis.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<int> decode_octal(const char* cp, unsigned char& out) noexcept {
    if (cp == nullptr || cp[0] != '\\') {
        return result_err<int>(Status::Invalid);
    }
    unsigned val = 0;
    for (int i = 0; i < 3; ++i) {
        const char c = cp[i + 1];
        if (c < '0' || c > '7') {
            return result_err<int>(Status::Invalid);
        }
        val = (val << 3) | static_cast<unsigned>(c - '0');
    }
    out = static_cast<unsigned char>(val);
    return result_ok(4);
}

[[nodiscard]] inline Result<int> decode_simple(const char* cp, unsigned char& out) noexcept {
    if (cp == nullptr || cp[0] != '\\') {
        return result_err<int>(Status::Invalid);
    }
    switch (cp[1]) {
    case 'n':
        out = '\n';
        return result_ok(2);
    case 'r':
        out = '\r';
        return result_ok(2);
    case 't':
        out = '\t';
        return result_ok(2);
    case '\\':
        out = '\\';
        return result_ok(2);
    default:
        return result_err<int>(Status::Invalid);
    }
}

} // namespace pbsd::userland::libc
