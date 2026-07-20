module;

export module pbsd.userland.netstat.inet;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/inet.c
export namespace pbsd::userland::usr_bin::netstat::inet {

[[nodiscard]] inline bool inet_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::inet
