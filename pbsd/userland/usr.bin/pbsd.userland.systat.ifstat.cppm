module;

export module pbsd.userland.systat.ifstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/ifstat.c
export namespace pbsd::userland::usr_bin::systat::ifstat {

[[nodiscard]] inline bool ifstat_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::ifstat
