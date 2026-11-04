module;

export module pbsd.userland.systat.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/main.c
export namespace pbsd::userland::usr_bin::systat::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::main
