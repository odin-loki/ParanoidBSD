module;

export module pbsd.userland.systat.icmp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/icmp.c
export namespace pbsd::userland::usr_bin::systat::icmp {

[[nodiscard]] inline bool icmp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::icmp
