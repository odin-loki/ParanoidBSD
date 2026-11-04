module;

export module pbsd.userland.locate.code.locate.code;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/locate/code/locate.code.c
export namespace pbsd::userland::usr_bin::locate::code::locate::code {

[[nodiscard]] inline bool code_locate_code_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::locate::code::locate::code
