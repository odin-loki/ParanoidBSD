module;

export module pbsd.userland.fstat.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fstat/main.c
export namespace pbsd::userland::usr_bin::fstat::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::fstat::main
