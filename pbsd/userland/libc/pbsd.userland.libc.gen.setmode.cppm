module;

export module pbsd.userland.libc.gen.setmode;

export import pbsd.core;

/// setmode from hbsd/src/lib/libc/gen/setmode.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool setmode_is_operator(char c) noexcept {
    return c == '+' || c == '-' || c == ' ';
}

} // namespace pbsd::userland::libc
