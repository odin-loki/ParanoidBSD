module;

export module pbsd.userland.systat.netstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/netstat.c
export namespace pbsd::userland::usr_bin::systat::netstat {

[[nodiscard]] inline bool netstat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::netstat
