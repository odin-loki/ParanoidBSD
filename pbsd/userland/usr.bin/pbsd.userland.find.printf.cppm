module;

export module pbsd.userland.find.printf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/find/printf.c
export namespace pbsd::userland::usr_bin::find::printf {

[[nodiscard]] inline bool printf_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::find::printf
