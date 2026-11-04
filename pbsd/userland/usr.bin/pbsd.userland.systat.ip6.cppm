module;

export module pbsd.userland.systat.ip6;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/ip6.c
export namespace pbsd::userland::usr_bin::systat::ip6 {

[[nodiscard]] inline bool ip6_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::ip6
