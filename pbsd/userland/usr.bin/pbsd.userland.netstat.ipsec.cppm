module;

export module pbsd.userland.netstat.ipsec;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/ipsec.c
export namespace pbsd::userland::usr_bin::netstat::ipsec {

[[nodiscard]] inline bool ipsec_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::ipsec
