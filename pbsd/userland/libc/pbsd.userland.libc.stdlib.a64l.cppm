module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.a64l;

/// a64l/l64a from hbsd/src/lib/libc/stdlib/{a64l,l64a}.c
export namespace pbsd::userland::libc {

inline constexpr int kADot = 46;
inline constexpr int kASlash = 47;
inline constexpr int kA0 = 48;
inline constexpr int kAA = 65;
inline constexpr int kAa = 97;

[[nodiscard]] inline long a64l(const char* s) noexcept {
    long value = 0;
    int shift = 0;
    for (int i = 0; s[i] != '\0' && i < 6; ++i) {
        int digit;
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c <= static_cast<unsigned char>(kASlash)) {
            digit = static_cast<int>(c) - kASlash + 1;
        } else if (c <= static_cast<unsigned char>(kA0 + 9)) {
            digit = static_cast<int>(c) - kA0 + 2;
        } else if (c <= static_cast<unsigned char>(kAA + 25)) {
            digit = static_cast<int>(c) - kAA + 12;
        } else {
            digit = static_cast<int>(c) - kAa + 38;
        }
        value |= static_cast<long>(digit) << shift;
        shift += 6;
    }
    return value;
}

[[nodiscard]] inline char* l64a(long value, char* buf, std::size_t buflen) noexcept {
    static const char digits[] =
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::size_t i = 0;
    do {
        if (i + 1 >= buflen) {
            break;
        }
        buf[i++] = digits[value & 0x3f];
        value >>= 6;
    } while (value != 0 && i < 6);
    if (i < buflen) {
        buf[i] = '\0';
    }
    return buf;
}

} // namespace pbsd::userland::libc
