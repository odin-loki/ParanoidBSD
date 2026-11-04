module;

export module pbsd.userland.procstat.procstat_sigs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_sigs.c
export namespace pbsd::userland::usr_bin::procstat::procstat_sigs {

[[nodiscard]] inline bool procstat_sigs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_sigs
