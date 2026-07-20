module;

export module pbsd.userland.netstat.pfkey;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/pfkey.c
export namespace pbsd::userland::usr_bin::netstat::pfkey {

[[nodiscard]] inline bool pfkey_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::pfkey
