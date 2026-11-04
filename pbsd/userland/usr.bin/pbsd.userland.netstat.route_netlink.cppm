module;

export module pbsd.userland.netstat.route_netlink;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/route_netlink.c
export namespace pbsd::userland::usr_bin::netstat::route_netlink {

[[nodiscard]] inline bool route_netlink_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::route_netlink
