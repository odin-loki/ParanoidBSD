module;

export module pbsd.userland.netstat.mroute6;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/mroute6.c
export namespace pbsd::userland::usr_bin::netstat::mroute6 {

[[nodiscard]] inline bool mroute6_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::mroute6
