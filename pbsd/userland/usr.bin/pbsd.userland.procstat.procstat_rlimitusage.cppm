module;

export module pbsd.userland.procstat.procstat_rlimitusage;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_rlimitusage.c
export namespace pbsd::userland::usr_bin::procstat::procstat_rlimitusage {

[[nodiscard]] inline bool procstat_rlimitusage_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_rlimitusage
