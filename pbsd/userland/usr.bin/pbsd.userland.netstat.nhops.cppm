module;

export module pbsd.userland.netstat.nhops;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/nhops.c
export namespace pbsd::userland::usr_bin::netstat::nhops {

[[nodiscard]] inline bool nhops_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::nhops
