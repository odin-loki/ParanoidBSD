module;

export module pbsd.userland.libc.locale.toupper;

export import pbsd.core;

/// toupper from hbsd/src/lib/libc/locale/toupper.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int toupper_ascii(int c) noexcept {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

} // namespace pbsd::userland::libc
