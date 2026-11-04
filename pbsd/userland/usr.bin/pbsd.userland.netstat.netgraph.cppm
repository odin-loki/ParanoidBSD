module;

export module pbsd.userland.netstat.netgraph;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/netgraph.c
export namespace pbsd::userland::usr_bin::netstat::netgraph {

[[nodiscard]] inline bool netgraph_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::netgraph
