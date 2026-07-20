module;

export module pbsd.userland.netstat.inet6;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/inet6.c
export namespace pbsd::userland::usr_bin::netstat::inet6 {

[[nodiscard]] inline bool inet6_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::inet6
