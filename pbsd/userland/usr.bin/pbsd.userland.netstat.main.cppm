module;

export module pbsd.userland.netstat.main;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/main.c
export namespace pbsd::userland::usr_bin::netstat::main {

[[nodiscard]] inline bool main_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::main
