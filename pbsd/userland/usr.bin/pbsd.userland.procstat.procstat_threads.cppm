module;

export module pbsd.userland.procstat.procstat_threads;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/procstat_threads.c
export namespace pbsd::userland::usr_bin::procstat::procstat_threads {

[[nodiscard]] inline bool procstat_threads_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::procstat_threads
