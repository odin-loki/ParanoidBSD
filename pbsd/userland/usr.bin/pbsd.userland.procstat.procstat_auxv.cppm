module;

export module pbsd.userland.procstat.procstat_auxv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_auxv.c
export namespace pbsd::userland::usr_bin::procstat::procstat_auxv {

[[nodiscard]] inline bool procstat_auxv_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_auxv
