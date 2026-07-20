module;

export module pbsd.userland.systat.icmp6;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/icmp6.c
export namespace pbsd::userland::usr_bin::systat::icmp6 {

[[nodiscard]] inline bool icmp6_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::icmp6
