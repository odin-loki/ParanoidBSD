module;

export module pbsd.userland.m4.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/m4/main.c
export namespace pbsd::userland::usr_bin::m4::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::m4::main
