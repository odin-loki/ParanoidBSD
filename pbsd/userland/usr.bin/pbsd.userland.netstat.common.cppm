module;

export module pbsd.userland.netstat.common;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/netstat/common.c
export namespace pbsd::userland::usr_bin::netstat::common {

[[nodiscard]] inline bool common_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::netstat::common
