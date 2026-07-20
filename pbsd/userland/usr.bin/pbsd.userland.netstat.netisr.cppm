module;

export module pbsd.userland.netstat.netisr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/netisr.c
export namespace pbsd::userland::usr_bin::netstat::netisr {

[[nodiscard]] inline bool netisr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::netisr
