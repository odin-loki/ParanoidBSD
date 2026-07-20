module;

export module pbsd.userland.netstat.mbuf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/mbuf.c
export namespace pbsd::userland::usr_bin::netstat::mbuf {

[[nodiscard]] inline bool mbuf_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::mbuf
