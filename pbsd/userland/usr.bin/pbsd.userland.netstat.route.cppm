module;

export module pbsd.userland.netstat.route;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/route.c
export namespace pbsd::userland::usr_bin::netstat::route {

[[nodiscard]] inline bool route_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::route
