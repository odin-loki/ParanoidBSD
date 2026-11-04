module;

export module pbsd.userland.procstat.procstat_args;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_args.c
export namespace pbsd::userland::usr_bin::procstat::procstat_args {

[[nodiscard]] inline bool procstat_args_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_args
