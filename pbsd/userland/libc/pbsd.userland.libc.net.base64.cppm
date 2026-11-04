module;
#include <cstddef>

export module pbsd.userland.libc.net.base64;

export import pbsd.core;

/// base64 from hbsd/src/lib/libc/net/base64.c
export namespace pbsd::userland::libc {

inline constexpr char kBase64Pad = '=';

[[nodiscard]] inline std::size_t base64_encoded_len(std::size_t raw_len) noexcept {
    return ((raw_len + 2) / 3) * 4;
}

[[nodiscard]] inline bool base64_valid_char(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
           c == '+' || c == '/' || c == kBase64Pad;
}

} // namespace pbsd::userland::libc
