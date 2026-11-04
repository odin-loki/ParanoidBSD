module;

export module pbsd.userland.netstat.mroute;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/mroute.c
export namespace pbsd::userland::usr_bin::netstat::mroute {

[[nodiscard]] inline bool mroute_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::mroute
