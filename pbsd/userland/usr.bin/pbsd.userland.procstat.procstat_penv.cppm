module;

export module pbsd.userland.procstat.procstat_penv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_penv.c
export namespace pbsd::userland::usr_bin::procstat::procstat_penv {

[[nodiscard]] inline bool procstat_penv_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_penv
