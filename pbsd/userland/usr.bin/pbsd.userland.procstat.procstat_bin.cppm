module;

export module pbsd.userland.procstat.procstat_bin;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_bin.c
export namespace pbsd::userland::usr_bin::procstat::procstat_bin {

[[nodiscard]] inline bool procstat_bin_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_bin
