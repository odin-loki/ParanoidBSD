module;

export module pbsd.userland.libc.locale.tolower;

export import pbsd.core;

/// tolower from hbsd/src/lib/libc/locale/tolower.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int tolower_ascii(int c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

} // namespace pbsd::userland::libc
