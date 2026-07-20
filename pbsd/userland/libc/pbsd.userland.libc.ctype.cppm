export module pbsd.userland.libc.ctype;

export import pbsd.core;

/// Wave 2 — ctype.h subset (ASCII C locale) from hbsd/src/lib/libc/locale.
export namespace pbsd::userland::libc::ctype {

[[nodiscard]] constexpr bool isascii_c(int c) noexcept {
    return c >= 0 && c <= 0x7f;
}

[[nodiscard]] constexpr bool isdigit_c(int c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool isxdigit_c(int c) noexcept {
    return isdigit_c(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

[[nodiscard]] constexpr bool isalpha_c(int c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

[[nodiscard]] constexpr bool isalnum_c(int c) noexcept {
    return isalpha_c(c) || isdigit_c(c);
}

[[nodiscard]] constexpr bool isspace_c(int c) noexcept {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

[[nodiscard]] constexpr bool isblank_c(int c) noexcept {
    return c == ' ' || c == '\t';
}

[[nodiscard]] constexpr bool isupper_c(int c) noexcept {
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]] constexpr bool islower_c(int c) noexcept {
    return c >= 'a' && c <= 'z';
}

[[nodiscard]] constexpr bool isprint_c(int c) noexcept {
    return c >= 0x20 && c <= 0x7e;
}

[[nodiscard]] constexpr bool isgraph_c(int c) noexcept {
    return c > 0x20 && c <= 0x7e;
}

[[nodiscard]] constexpr bool iscntrl_c(int c) noexcept {
    return (c >= 0 && c < 0x20) || c == 0x7f;
}

[[nodiscard]] constexpr bool ispunct_c(int c) noexcept {
    return isgraph_c(c) && !isalnum_c(c);
}

[[nodiscard]] constexpr int toupper_c(int c) noexcept {
    return islower_c(c) ? c - 'a' + 'A' : c;
}

[[nodiscard]] constexpr int tolower_c(int c) noexcept {
    return isupper_c(c) ? c - 'A' + 'a' : c;
}

} // namespace pbsd::userland::libc::ctype
