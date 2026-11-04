module;

export module pbsd.userland.systat.ip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/ip.c
export namespace pbsd::userland::usr_bin::systat::ip {

[[nodiscard]] inline bool ip_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::ip
