module;

export module pbsd.userland.procstat.procstat_cs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_cs.c
export namespace pbsd::userland::usr_bin::procstat::procstat_cs {

[[nodiscard]] inline bool procstat_cs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_cs
