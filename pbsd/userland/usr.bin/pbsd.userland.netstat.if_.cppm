module;

export module pbsd.userland.netstat.if_;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/if.c
export namespace pbsd::userland::usr_bin::netstat::if_ {

[[nodiscard]] inline bool if__flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::if_
