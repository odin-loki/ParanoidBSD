module;

export module pbsd.userland.procstat.procstat_rlimit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_rlimit.c
export namespace pbsd::userland::usr_bin::procstat::procstat_rlimit {

[[nodiscard]] inline bool procstat_rlimit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_rlimit
