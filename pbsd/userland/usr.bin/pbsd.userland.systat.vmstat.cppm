module;

export module pbsd.userland.systat.vmstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/vmstat.c
export namespace pbsd::userland::usr_bin::systat::vmstat {

[[nodiscard]] inline bool vmstat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::vmstat
